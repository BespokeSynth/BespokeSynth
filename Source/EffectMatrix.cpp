#include "EffectMatrix.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FileStream.h"
#include "IAudioReceiver.h"
#include "UIControlMacros.h"
#include <cmath>

EffectMatrix::EffectMatrix()
: IAudioProcessor(gBufferSize)
, mOutputBuffer(gBufferSize)
{
   ClearGrid();

   mPitchBuffer0.resize(PITCH_BUF_SIZE, 0.0f);
   mPitchBuffer1.resize(PITCH_BUF_SIZE, 0.0f);
   mStutterBuffer0.resize(STUTTER_BUF_SIZE, 0.0f);
   mStutterBuffer1.resize(STUTTER_BUF_SIZE, 0.0f);
   mRevBuf1L.resize(REV_DEL_1, 0.0f); mRevBuf1R.resize(REV_DEL_1, 0.0f);
   mRevBuf2L.resize(REV_DEL_2, 0.0f); mRevBuf2R.resize(REV_DEL_2, 0.0f);
   mRevBuf3L.resize(REV_DEL_3, 0.0f); mRevBuf3R.resize(REV_DEL_3, 0.0f);
   mDelayBuffer0.resize(DELAY_BUF_SIZE, 0.0f);
   mDelayBuffer1.resize(DELAY_BUF_SIZE, 0.0f);
}

EffectMatrix::~EffectMatrix()
{
}

void EffectMatrix::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mMixSlider, "mix", &mWetDry, 0.0f, 1.0f);
   UIBLOCK_NEWLINE();
   DROPDOWN(mQuantizeLengthSelector, "interval", (int*)(&mQuantizeInterval), 70);
   mQuantizeLengthSelector->AddLabel("32n", kInterval_32n);
   mQuantizeLengthSelector->AddLabel("16nt", kInterval_16nt);
   mQuantizeLengthSelector->AddLabel("16n", kInterval_16n);
   mQuantizeLengthSelector->AddLabel("8nt", kInterval_8nt);
   mQuantizeLengthSelector->AddLabel("8n", kInterval_8n);
   mQuantizeLengthSelector->AddLabel("4nt", kInterval_4nt);
   mQuantizeLengthSelector->AddLabel("4n", kInterval_4n);
   mQuantizeLengthSelector->AddLabel("2n", kInterval_2n);
   mQuantizeLengthSelector->AddLabel("1", kInterval_1n);
   UIBLOCK_NEWLINE();
   DROPDOWN(mPresetSelector, "preset", (int*)(&mCurrentPreset), 100);
   mPresetSelector->AddLabel("custom", kPreset_Custom);
   mPresetSelector->AddLabel("clear", kPreset_Clear);
   mPresetSelector->AddLabel("glitch stutter", kPreset_GlitchStutter);
   mPresetSelector->AddLabel("bitcrush drop", kPreset_BitcrushDrop);
   mPresetSelector->AddLabel("filter riser", kPreset_FilterRiser);
   mPresetSelector->AddLabel("chaos random", kPreset_ChaosRandom);
   mPresetSelector->AddLabel("dense chaos", kPreset_DenseChaos);
   mPresetSelector->AddLabel("rhythmic echo", kPreset_RhythmicEcho);
   mPresetSelector->AddLabel("ambient wash", kPreset_AmbientWash);
   mPresetSelector->AddLabel("stepped riser", kPreset_SteppedRiser);
   UIBLOCK_NEWLINE();
   BUTTON(mClearButton, "clear grid");
   BUTTON(mRandomButton, "randomize");
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mRandomDensitySlider, "density", &mRandomDensity, 0.0f, 1.0f);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mRandomIntensitySlider, "intensity", &mRandomIntensity, 0.0f, 1.0f);
   UIBLOCK_NEWLINE();

   ENDUIBLOCK(mWidth, mHeight);

   // Position 16x7 grid arena (leave room for labels + [R] buttons on left)
   mArenaX = mWidth + 95;
   mArenaY = 10;
   mArenaW = 460;
   mArenaH = 230;

   // Create per-row randomize buttons positioned to the left of each grid row
   float cellH = mArenaH / (float)NUM_FX;
   static const char* rowBtnNames[NUM_FX] = { "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
   for (int row = 0; row < NUM_FX; ++row)
   {
      float btnX = mArenaX - 20;
      float btnY = mArenaY + row * cellH + cellH / 2 - 6;
      mRowRandomButtons[row] = new ClickButton(this, rowBtnNames[row], btnX, btnY);
   }

   mWidth += mArenaW + 30;
   mHeight = MAX(mHeight, mArenaH + 20);
}

void EffectMatrix::Process(double time)
{
   PROFILER(EffectMatrix);

   if (!mEnabled)
      return;

   ComputeSliders(0);

   SyncBuffers();

   ChannelBuffer* inputBuffer = GetBuffer();
   int numChannels = inputBuffer->NumActiveChannels();
   int bufferSize = inputBuffer->BufferSize();

   if (numChannels == 0)
   {
      inputBuffer->Reset();
      return;
   }

   mOutputBuffer.SetNumActiveChannels(numChannels);
   mOutputBuffer.Clear();

   // Transport tracking for 16-step grid
   double stepDuration = TheTransport->GetMeasureFraction(mQuantizeInterval);
   if (stepDuration <= 0.0001) stepDuration = 0.125;

   double measureTime = TheTransport->GetMeasureTime(time) - mPlayStartTime;
   if (measureTime < 0) measureTime = 0;

   double patternDuration = stepDuration * NUM_STEPS;
   double progress = fmod(measureTime / patternDuration, 1.0);
   if (progress < 0.0) progress = 0.0;

   int step = (int)(progress * NUM_STEPS);
   if (step >= NUM_STEPS) step = NUM_STEPS - 1;
   if (step < 0) step = 0;

   mCurrentStep = step;
   mPlayheadProgress = (float)progress;

   // Active step effects
   float bitcrushVal = mGrid[kFx_Bitcrush][step];
   float filterVal   = mGrid[kFx_Filter][step];
   float pitchVal    = mGrid[kFx_Pitch][step];
   float stutterVal  = mGrid[kFx_Stutter][step];
   float reverbVal   = mGrid[kFx_Reverb][step];
   float gateVal     = mGrid[kFx_Gate][step];
   float delayVal    = mGrid[kFx_Delay][step];

   float* inL = inputBuffer->GetChannel(0);
   float* inR = numChannels > 1 ? inputBuffer->GetChannel(1) : inputBuffer->GetChannel(0);
   float* outL = mOutputBuffer.GetChannel(0);
   float* outR = numChannels > 1 ? mOutputBuffer.GetChannel(1) : mOutputBuffer.GetChannel(0);

   for (int i = 0; i < bufferSize; ++i)
   {
      float dryL = inL[i];
      float dryR = inR[i];

      float wetL = dryL;
      float wetR = dryR;

      // 1. Bitcrusher
      if (bitcrushVal > 0.05f)
      {
         int rate = 1 + (int)(bitcrushVal * 12.0f);
         if (++mBitcrushCounter >= rate)
         {
            mBitcrushCounter = 0;
            float bits = 16.0f - bitcrushVal * 12.0f;
            float levels = powf(2.0f, bits);
            mBitcrushHeld[0] = floorf(wetL * levels + 0.5f) / levels;
            mBitcrushHeld[1] = floorf(wetR * levels + 0.5f) / levels;
         }
         wetL = mBitcrushHeld[0];
         wetR = mBitcrushHeld[1];
      }

      // 2. Filter (State Variable LPF)
      if (filterVal > 0.05f)
      {
         float cutoff = 300.0f + (1.0f - filterVal) * 9000.0f;
         float f = 2.0f * sinf(PI * cutoff / gSampleRate);
         float q = 0.5f;

         mFilterLow[0] += f * mFilterBand[0];
         float highL = wetL - mFilterLow[0] - q * mFilterBand[0];
         mFilterBand[0] += f * highL;

         mFilterLow[1] += f * mFilterBand[1];
         float highR = wetR - mFilterLow[1] - q * mFilterBand[1];
         mFilterBand[1] += f * highR;

         wetL = mFilterLow[0];
         wetR = mFilterLow[1];
      }

      // 3. Pitch Shift (Granular pitch delay)
      if (pitchVal > 0.05f)
      {
         mPitchBuffer0[mPitchWritePos] = wetL;
         mPitchBuffer1[mPitchWritePos] = wetR;

         float shiftRate = 1.0f + pitchVal * 1.0f; // up to +1 octave
         mPitchReadPos[0] += shiftRate;
         mPitchReadPos[1] += shiftRate;

         if (mPitchReadPos[0] >= PITCH_BUF_SIZE) mPitchReadPos[0] -= (PITCH_BUF_SIZE / 2);
         if (mPitchReadPos[1] >= PITCH_BUF_SIZE) mPitchReadPos[1] -= (PITCH_BUF_SIZE / 2);

         int rIdx0 = ((int)mPitchReadPos[0]) % PITCH_BUF_SIZE;
         int rIdx1 = ((int)mPitchReadPos[1]) % PITCH_BUF_SIZE;

         wetL = mPitchBuffer0[rIdx0];
         wetR = mPitchBuffer1[rIdx1];

         mPitchWritePos = (mPitchWritePos + 1) % PITCH_BUF_SIZE;
      }

      // 4. Stutter / Freeze
      if (stutterVal > 0.05f)
      {
         mStutterBuffer0[mStutterWritePos] = wetL;
         mStutterBuffer1[mStutterWritePos] = wetR;

         int stutterLen = (int)(gSampleRate * 0.125f * (1.1f - stutterVal));
         if (stutterLen < 256) stutterLen = 256;

         mStutterReadPos = (mStutterReadPos + 1) % stutterLen;
         int readIdx = (mStutterWritePos - stutterLen + mStutterReadPos + STUTTER_BUF_SIZE) % STUTTER_BUF_SIZE;

         wetL = mStutterBuffer0[readIdx];
         wetR = mStutterBuffer1[readIdx];

         mStutterWritePos = (mStutterWritePos + 1) % STUTTER_BUF_SIZE;
      }

      // 5. Reverb (Comb Filter feedback)
      if (reverbVal > 0.05f)
      {
         float feedback = 0.7f * reverbVal;

         mRevBuf1L[mRevIdx1] = wetL + mRevBuf1L[mRevIdx1] * feedback;
         mRevBuf2L[mRevIdx2] = wetL + mRevBuf2L[mRevIdx2] * feedback;
         mRevBuf3L[mRevIdx3] = wetL + mRevBuf3L[mRevIdx3] * feedback;

         mRevBuf1R[mRevIdx1] = wetR + mRevBuf1R[mRevIdx1] * feedback;
         mRevBuf2R[mRevIdx2] = wetR + mRevBuf2R[mRevIdx2] * feedback;
         mRevBuf3R[mRevIdx3] = wetR + mRevBuf3R[mRevIdx3] * feedback;

         float revL = (mRevBuf1L[mRevIdx1] + mRevBuf2L[mRevIdx2] + mRevBuf3L[mRevIdx3]) * 0.33f;
         float revR = (mRevBuf1R[mRevIdx1] + mRevBuf2R[mRevIdx2] + mRevBuf3R[mRevIdx3]) * 0.33f;

         wetL = wetL * 0.5f + revL * 0.5f;
         wetR = wetR * 0.5f + revR * 0.5f;

         mRevIdx1 = (mRevIdx1 + 1) % REV_DEL_1;
         mRevIdx2 = (mRevIdx2 + 1) % REV_DEL_2;
         mRevIdx3 = (mRevIdx3 + 1) % REV_DEL_3;
      }

      // 6. Gate / Tremolo
      if (gateVal > 0.05f)
      {
         float gateMult = 1.0f - gateVal;
         wetL *= gateMult;
         wetR *= gateMult;
      }

      // 7. Delay
      if (delayVal > 0.05f)
      {
         int delayTimeSamples = (int)(gSampleRate * (0.5f - delayVal * 0.45f));
         if (delayTimeSamples < 10) delayTimeSamples = 10;
         if (delayTimeSamples >= DELAY_BUF_SIZE) delayTimeSamples = DELAY_BUF_SIZE - 1;

         int readPos = (mDelayWritePos - delayTimeSamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
         
         float dl = mDelayBuffer0[readPos];
         float dr = mDelayBuffer1[readPos];

         float feedback = delayVal * 0.8f;
         mDelayBuffer0[mDelayWritePos] = wetL + dl * feedback;
         mDelayBuffer1[mDelayWritePos] = wetR + dr * feedback;

         wetL = wetL * 0.5f + dl * 0.5f;
         wetR = wetR * 0.5f + dr * 0.5f;

         mDelayWritePos = (mDelayWritePos + 1) % DELAY_BUF_SIZE;
      }
      else
      {
         mDelayBuffer0[mDelayWritePos] = wetL;
         mDelayBuffer1[mDelayWritePos] = wetR;
         mDelayWritePos = (mDelayWritePos + 1) % DELAY_BUF_SIZE;
      }

      // Mix wet/dry
      outL[i] = dryL * (1.0f - mWetDry) + wetL * mWetDry;
      outR[i] = dryR * (1.0f - mWetDry) + wetR * mWetDry;
   }

   // Output to targets
   IAudioReceiver* targetOut = GetTarget(0);
   if (targetOut)
   {
      for (int ch = 0; ch < mOutputBuffer.NumActiveChannels(); ++ch)
      {
         Add(targetOut->GetBuffer()->GetChannel(ch), mOutputBuffer.GetChannel(ch), bufferSize);
      }
   }
   GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(0), bufferSize, 0);
   if (mOutputBuffer.NumActiveChannels() > 1)
      GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(1), bufferSize, 1);

   inputBuffer->Reset();
}

void EffectMatrix::Poll()
{
}

void EffectMatrix::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   // Draw UI controls
   mMixSlider->Draw();
   mQuantizeLengthSelector->Draw();
   mPresetSelector->Draw();
   mClearButton->Draw();
   mRandomButton->Draw();
   mRandomDensitySlider->Draw();
   mRandomIntensitySlider->Draw();

   // Draw Arena Box
   ofPushStyle();
   ofSetColor(30, 30, 35, 255);
   ofFill();
   ofRect(mArenaX, mArenaY, mArenaW, mArenaH);

   ofSetColor(200, 200, 200, 255);
   ofNoFill();
   ofRect(mArenaX, mArenaY, mArenaW, mArenaH);

   // Grid dimensions
   float cellW = mArenaW / (float)NUM_STEPS;
   float cellH = mArenaH / (float)NUM_FX;

   static const char* fxNames[NUM_FX] = {
      "BITCRUSH", "FILTER", "PITCH", "STUTTER", "REVERB", "GATE", "DELAY"
   };

   static const ofColor fxColors[NUM_FX] = {
      ofColor(255, 60, 120),  // Bitcrush: Pink
      ofColor(40, 180, 255),  // Filter: Blue
      ofColor(255, 150, 30),  // Pitch: Orange
      ofColor(80, 230, 100),  // Stutter: Lime Green
      ofColor(170, 70, 240),  // Reverb: Purple
      ofColor(0, 230, 230),   // Gate: Cyan
      ofColor(255, 255, 0)    // Delay: Yellow
   };

   // Draw grid cells
   for (int row = 0; row < NUM_FX; ++row)
   {
      for (int col = 0; col < NUM_STEPS; ++col)
      {
         float cx = mArenaX + col * cellW;
         float cy = mArenaY + row * cellH;

         float val = mGrid[row][col];
         if (val > 0.01f)
         {
            // Bar grows from BOTTOM up — 100% val = full cell, 10% val = tiny bar at bottom
            float barH = val * (cellH - 2);
            float barY = cy + 1 + (cellH - 2) - barH;
            ofColor c = fxColors[row];
            c.a = 220;
            ofSetColor(c);
            ofFill();
            ofRect(cx + 1, barY, cellW - 2, barH);
         }

         // Cell outline
         ofSetColor(60, 60, 65);
         ofNoFill();
         ofRect(cx, cy, cellW, cellH);
      }

      // Row label to the left of the grid, aligned next to the buttons
      ofSetColor(255, 255, 255, 255);
      DrawTextRightJustify(fxNames[row], mArenaX - 25, mArenaY + row * cellH + cellH / 2 + 4, 10);

      // Draw per-row randomize button
      mRowRandomButtons[row]->Draw();
   }

   // Draw playhead vertical line
   if (mPlayheadProgress >= 0.0f && mPlayheadProgress <= 1.0f)
   {
      float px = mArenaX + mPlayheadProgress * mArenaW;
      ofSetColor(255, 255, 255, 220);
      ofSetLineWidth(2.0f);
      ofLine(px, mArenaY, px, mArenaY + mArenaH);
      ofSetLineWidth(1.0f);
   }

   ofPopStyle();
}

void EffectMatrix::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   float rx = x - mArenaX;
   float ry = y - mArenaY;

   if (rx >= 0 && rx <= mArenaW && ry >= 0 && ry <= mArenaH)
   {
      float cellW = mArenaW / (float)NUM_STEPS;
      float cellH = mArenaH / (float)NUM_FX;

      int col = (int)(rx / cellW);
      int row = (int)(ry / cellH);
      col = ofClamp(col, 0, NUM_STEPS - 1);
      row = ofClamp(row, 0, NUM_FX - 1);

      mIsDraggingCell = true;
      mDragRow = row;
      mDragCol = col;
      mDragStartY = y;

      if (right)
      {
         mGrid[row][col] = 0.0f;
         mIsDraggingCell = false; // no drag needed for right click erase
      }
      else
      {
         if (mGrid[row][col] < 0.05f) 
            mGrid[row][col] = 1.0f;
         mDragStartValue = mGrid[row][col];
      }

      mCurrentPreset = kPreset_Custom;
      mPresetSelector->SetValue(kPreset_Custom, 0);
   }
}

bool EffectMatrix::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (!mIsDraggingCell) return false;

   float deltaY = mDragStartY - y; // moving up is positive delta
   float newVal = ofClamp(mDragStartValue + deltaY * 0.01f, 0.0f, 1.0f);
   mGrid[mDragRow][mDragCol] = newVal;

   mCurrentPreset = kPreset_Custom;
   mPresetSelector->SetValue(kPreset_Custom, 0);

   return true;
}

void EffectMatrix::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mIsDraggingCell = false;
}

void EffectMatrix::Resize(float w, float h)
{
}

void EffectMatrix::ClearGrid()
{
   for (int row = 0; row < NUM_FX; ++row)
   {
      for (int col = 0; col < NUM_STEPS; ++col)
         mGrid[row][col] = 0.0f;
   }
}

void EffectMatrix::RandomizeGrid(int rowToRandomize)
{
   if (rowToRandomize == -1)
      ClearGrid();
   else
   {
      for (int col = 0; col < NUM_STEPS; ++col)
         mGrid[rowToRandomize][col] = 0.0f;
   }

   float density = mRandomDensity;
   float intensity = mRandomIntensity;

   // Scale density so 1.0 = 85% fill, 0.0 = 0% fill
   float fillChance = density * 0.85f;

   int rowStart = (rowToRandomize == -1) ? 0 : rowToRandomize;
   int rowEnd   = (rowToRandomize == -1) ? NUM_FX : rowToRandomize + 1;

   for (int row = rowStart; row < rowEnd; ++row)
   {
      for (int col = 0; col < NUM_STEPS; ++col)
      {
         if (ofRandom(1.0f) < fillChance)
         {
            float val = intensity + ofRandom(-0.2f, 0.2f);
            mGrid[row][col] = ofClamp(val, 0.1f, 1.0f);
         }
      }
   }
}

void EffectMatrix::ApplyPreset(FxPreset preset)
{
   ClearGrid();
   switch (preset)
   {
      case kPreset_Clear:
         break;
      case kPreset_GlitchStutter:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            if (i % 4 == 3) mGrid[kFx_Stutter][i] = 0.8f;
            if (i % 8 == 7) mGrid[kFx_Bitcrush][i] = 1.0f;
         }
         break;
      case kPreset_BitcrushDrop:
         for (int i = 8; i < NUM_STEPS; ++i)
         {
            mGrid[kFx_Bitcrush][i] = (float)(i - 8) / 8.0f;
            if (i % 2 == 1) mGrid[kFx_Gate][i] = 0.9f;
         }
         break;
      case kPreset_FilterRiser:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            mGrid[kFx_Filter][i] = (float)i / (NUM_STEPS - 1);
            if (i >= 12) mGrid[kFx_Reverb][i] = 0.7f;
         }
         break;
      case kPreset_ChaosRandom:
         RandomizeGrid();
         break;
      case kPreset_DenseChaos:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            if (ofRandom(1.0f) < 0.8f) mGrid[(int)ofRandom(0, NUM_FX)][i] = ofRandom(0.7f, 1.0f);
         }
         break;
      case kPreset_RhythmicEcho:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            if (i % 4 == 0) mGrid[kFx_Stutter][i] = 0.5f;
            if (i % 3 == 0) mGrid[kFx_Delay][i] = 0.8f;
         }
         break;
      case kPreset_AmbientWash:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            if (i % 8 == 0) { mGrid[kFx_Reverb][i] = 1.0f; mGrid[kFx_Filter][i] = 0.3f; }
            if (i % 4 == 2) { mGrid[kFx_Pitch][i] = 0.6f; }
         }
         break;
      case kPreset_SteppedRiser:
         for (int i = 0; i < NUM_STEPS; ++i)
         {
            mGrid[kFx_Filter][i] = (i / (float)NUM_STEPS);
            if (i > 8) mGrid[kFx_Bitcrush][i] = (i - 8) / 8.0f;
         }
         break;
      default:
         break;
   }
}

void EffectMatrix::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mPresetSelector)
   {
      ApplyPreset(mCurrentPreset);
   }
}

void EffectMatrix::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
   {
      ClearGrid();
      mCurrentPreset = kPreset_Clear;
      mPresetSelector->SetValue(kPreset_Clear, 0);
   }
   else if (button == mRandomButton)
   {
      RandomizeGrid();
      mCurrentPreset = kPreset_Custom;
      mPresetSelector->SetValue(kPreset_Custom, 0);
   }
   else
   {
      for (int row = 0; row < NUM_FX; ++row)
      {
         if (button == mRowRandomButtons[row])
         {
            RandomizeGrid(row);
            mCurrentPreset = kPreset_Custom;
            mPresetSelector->SetValue(kPreset_Custom, 0);
            break;
         }
      }
   }
}

void EffectMatrix::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void EffectMatrix::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void EffectMatrix::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void EffectMatrix::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWetDry;
   out << (int)mQuantizeInterval;
   out << (int)mCurrentPreset;

   for (int row = 0; row < NUM_FX; ++row)
   {
      for (int col = 0; col < NUM_STEPS; ++col)
         out << mGrid[row][col];
   }
}

void EffectMatrix::LoadState(FileStreamIn& in, int rev)
{
   if (rev >= 3)
   {
      IDrawableModule::LoadState(in, rev);
   }

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWetDry;
   mMixSlider->SetValue(mWetDry, 0);

   int intervalVal;
   in >> intervalVal;
   mQuantizeInterval = (NoteInterval)intervalVal;
   mQuantizeLengthSelector->SetValue(mQuantizeInterval, 0);

   int presetVal;
   in >> presetVal;
   mCurrentPreset = (FxPreset)presetVal;
   mPresetSelector->SetValue(mCurrentPreset, 0);

   int numRows = (rev <= 1) ? 6 : NUM_FX;
   for (int row = 0; row < numRows; ++row)
   {
      for (int col = 0; col < NUM_STEPS; ++col)
      {
         if (row < NUM_FX)
            in >> mGrid[row][col];
         else
         {
            float dummy;
            in >> dummy;
         }
      }
   }
}

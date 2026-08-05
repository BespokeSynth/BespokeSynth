/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  DiceSynth.cpp
//  Bespoke
//

#include "DiceSynth.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "FFT.h"
#include "BiquadFilter.h"
#include "ModuleContainer.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cmath>

namespace
{
   //Layout is grouped by what a control ACTS ON, not by when it was added: the mold mode sits at the
   //top because it changes the meaning of everything under it, then everything that shapes the
   //SAMPLE, then a divider, then everything that drives the live FX pad.
   const float kModuleW = 320;
   const float kCtrlH = 18;
   const float kCtrlGap = 6;
   const float kDiceX = 6;

   //--- mode, at the very top ---
   const float kModeY = 4;
   const float kModeW = 84;

   //--- sample section ---
   const float kDiceY = kModeY + kCtrlH + 8;
   const float kDiceSize = 80;

   const float kWaveX = kDiceX + kDiceSize + 10;
   const float kWaveY = kDiceY;
   const float kWaveW = kModuleW - kWaveX - 6;
   const float kWaveH = 80;

   //play / stop / loop / volume all share one row, same height, evenly spaced
   const float kCtrlY = kDiceY + kDiceSize + 8;
   const float kCtrlBtnW = 46;
   const float kLoopX = kDiceX + (kCtrlBtnW + kCtrlGap) * 2;
   const float kLoopW = 60;
   const float kVolX = kLoopX + kLoopW + kCtrlGap;

   //iteration acts on the SOURCE, so it belongs with the sample controls rather than the fx ones
   const float kIterY = kCtrlY + kCtrlH + 4;
   const float kIterW = 60;
   const float kResetX = kDiceX + kIterW + kCtrlGap;
   const float kResetW = 52;
   const float kGenTextX = kResetX + kResetW + 10;

   //--- divider, then the FX section ---
   const float kDividerY = kIterY + kCtrlH + 8;

   const float kFxRowY = kDividerY + 10;
   const float kFxMixX = kDiceX + 112;

   const float kPathCtrlY = kFxRowY + kCtrlH + 6;
   const float kPlayPathX = kDiceX + 110;
   const float kLoopPathX = kPlayPathX + 100;

   const float kQuantCtrlY = kPathCtrlY + kCtrlH + 6;
   const float kQuantIntervalX = kDiceX + 80;

   const float kXYPadX = 6;
   const float kXYPadY = kQuantCtrlY + kCtrlH + 8;
   const float kXYPadSize = kModuleW - 12; //fills the module width - the module itself is what got smaller

   //Catmull-Rom interpolated read. Reading a buffer at a fractional rate with plain integer
   //indexing is a zero-order hold, which is literally what a bitcrusher's sample-rate reduction
   //does - it was making every time-warped or pitch-shifted render sound gritty.
   float CubicInterp(const float* buf, int len, double pos)
   {
      if (len <= 0)
         return 0.0f;
      int i1 = (int)pos;
      if (i1 < 0)
         i1 = 0;
      if (i1 > len - 1)
         i1 = len - 1;
      float f = (float)(pos - (double)i1);
      int i0 = MAX(0, i1 - 1);
      int i2 = MIN(len - 1, i1 + 1);
      int i3 = MIN(len - 1, i1 + 2);
      float y0 = buf[i0], y1 = buf[i1], y2 = buf[i2], y3 = buf[i3];
      float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
      float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
      float c = -0.5f * y0 + 0.5f * y2;
      return ((a * f + b) * f + c) * f + y1;
   }

   //sub-bin peak location, so pitch tracking isn't quantized to the FFT's bin spacing
   float ParabolicPeakBin(const float* mag, int bin, int numBins)
   {
      if (bin <= 0 || bin >= numBins - 1)
         return (float)bin;
      float l = mag[bin - 1], c = mag[bin], r = mag[bin + 1];
      float denom = l - 2.0f * c + r;
      if (fabsf(denom) < 1e-9f)
         return (float)bin;
      return (float)bin + 0.5f * (l - r) / denom;
   }
}

//static
float DiceSynth::NoiseBandEdgeHz(int edgeIndex)
{
   const float lo = 40.0f, hi = 18000.0f;
   return lo * powf(hi / lo, (float)edgeIndex / (float)kNumNoiseBands);
}

DiceSynth::DiceSynth()
: mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mWriteBuffer.SetNumActiveChannels(2);

   mPitchBuf[0].resize(kPitchBufSize, 0.0f);
   mPitchBuf[1].resize(kPitchBufSize, 0.0f);
   mDelayBuf[0].resize(kDelayBufSize, 0.0f);
   mDelayBuf[1].resize(kDelayBufSize, 0.0f);
   mRevBuf1[0].resize(kRevDel1, 0.0f);
   mRevBuf1[1].resize(kRevDel1, 0.0f);
   mRevBuf2[0].resize(kRevDel2, 0.0f);
   mRevBuf2[1].resize(kRevDel2, 0.0f);
   mRevBuf3[0].resize(kRevDel3, 0.0f);
   mRevBuf3[1].resize(kRevDel3, 0.0f);
   mStutterBuf[0].resize(kStutterBufSize, 0.0f);
   mStutterBuf[1].resize(kStutterBufSize, 0.0f);
   mRepeaterBuf[0].resize(kRepeaterBufSize, 0.0f);
   mRepeaterBuf[1].resize(kRepeaterBufSize, 0.0f);
   mReverseBuf[0].resize(kReverseBufSize, 0.0f);
   mReverseBuf[1].resize(kReverseBufSize, 0.0f);
   mCombBuf[0].resize(kCombBufSize, 0.0f);
   mCombBuf[1].resize(kCombBufSize, 0.0f);
   mChorusBuf[0].resize(kChorusBufSize, 0.0f);
   mChorusBuf[1].resize(kChorusBufSize, 0.0f);

   //start the effects on an even circle around the pad - "randomize fx" scatters them from here
   for (int e = 0; e < kNumLiveEffects; ++e)
   {
      float angle = -(float)PI * 0.5f + e * ((float)TWO_PI / kNumLiveEffects);
      mEffectSpots[e].x = 0.5f + 0.42f * cosf(angle);
      mEffectSpots[e].y = 0.5f + 0.42f * sinf(angle);
   }
}

DiceSynth::~DiceSynth()
{
}

void DiceSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   //--- mode: first, because it decides what everything below it means ---
   mMoldModeSelector = new DropdownList(this, "mold", kDiceX, kModeY, &mMoldMode, kModeW);
   mMoldModeSelector->AddLabel("effects", kMold_Effects);
   mMoldModeSelector->AddLabel("resynth", kMold_Resynth);

   float chaosX = kDiceX + kModeW + kCtrlGap;
   mChaosSlider = new FloatSlider(this, "chaos", chaosX, kModeY, mWidth - chaosX - 6, kCtrlH, &mChaos, 0.0f, 1.0f);

   //--- sample controls ---
   mPlayButton = new ClickButton(this, "play", kDiceX, kCtrlY);
   mPlayButton->SetDimensions(kCtrlBtnW, kCtrlH);
   mStopButton = new ClickButton(this, "stop", kDiceX + kCtrlBtnW + kCtrlGap, kCtrlY);
   mStopButton->SetDimensions(kCtrlBtnW, kCtrlH);
   mLoopCheckbox = new Checkbox(this, "loop", kLoopX, kCtrlY + 2, &mLoop);
   mGainSlider = new FloatSlider(this, "volume", kVolX, kCtrlY, mWidth - kVolX - 6, kCtrlH, &mGain, 0.0f, 2.0f);


   mIterateButton = new ClickButton(this, "iterate", kDiceX, kIterY);
   mIterateButton->SetDimensions(kIterW, kCtrlH);
   mResetButton = new ClickButton(this, "reset", kResetX, kIterY);
   mResetButton->SetDimensions(kResetW, kCtrlH);

   //--- fx controls: everything that drives the live XY pad ---
   mRandomizeEffectsButton = new ClickButton(this, "randomize fx", kDiceX, kFxRowY);
   mRandomizeEffectsButton->SetDimensions(100, kCtrlH);
   mFxMixSlider = new FloatSlider(this, "fx mix", kFxMixX, kFxRowY, mWidth - kFxMixX - 6, kCtrlH, &mFxMix, 0.0f, 1.0f);

   mRecordPathCheckbox = new Checkbox(this, "record path", kDiceX, kPathCtrlY + 2, &mRecordingPath);
   mPlayPathCheckbox = new Checkbox(this, "play path", kPlayPathX, kPathCtrlY + 2, &mPlayingPath);
   mLoopPathCheckbox = new Checkbox(this, "loop path", kLoopPathX, kPathCtrlY + 2, &mLoopPath);

   mQuantizePathCheckbox = new Checkbox(this, "quantize", kDiceX, kQuantCtrlY + 2, &mQuantizePath);
   mQuantizeIntervalSelector = new DropdownList(this, "interval", kQuantIntervalX, kQuantCtrlY, (int*)&mQuantizeInterval, 60);
   mQuantizeIntervalSelector->AddLabel("8n", kInterval_8n);
   mQuantizeIntervalSelector->AddLabel("4n", kInterval_4n);
   mQuantizeIntervalSelector->AddLabel("2n", kInterval_2n);
   mQuantizeIntervalSelector->AddLabel("1", kInterval_1n);
   mQuantizeIntervalSelector->AddLabel("2", kInterval_2);
   mQuantizeIntervalSelector->AddLabel("3", kInterval_3);
   mQuantizeIntervalSelector->AddLabel("4", kInterval_4);
   mQuantizeIntervalSelector->AddLabel("8", kInterval_8);
   mQuantizeIntervalSelector->AddLabel("16", kInterval_16);
   mQuantizeIntervalSelector->AddLabel("32", kInterval_32);
   mQuantizeIntervalSelector->AddLabel("64", kInterval_64);
}


void DiceSynth::ResetLiveEffectState()
{
   mFilterLow[0] = mFilterLow[1] = 0.0f;
   mFilterBand[0] = mFilterBand[1] = 0.0f;
   std::fill(mCombBuf[0].begin(), mCombBuf[0].end(), 0.0f);
   std::fill(mCombBuf[1].begin(), mCombBuf[1].end(), 0.0f);
   mCombWritePos = 0;
   std::fill(mChorusBuf[0].begin(), mChorusBuf[0].end(), 0.0f);
   std::fill(mChorusBuf[1].begin(), mChorusBuf[1].end(), 0.0f);
   mChorusWritePos = 0;
   mChorusPhase = 0.0f;
   std::fill(mPitchBuf[0].begin(), mPitchBuf[0].end(), 0.0f);
   std::fill(mPitchBuf[1].begin(), mPitchBuf[1].end(), 0.0f);
   mPitchReadPos[0] = mPitchReadPos[1] = 0.0f;
   mFreqShiftPhase = 0.0f;
   std::fill(mDelayBuf[0].begin(), mDelayBuf[0].end(), 0.0f);
   std::fill(mDelayBuf[1].begin(), mDelayBuf[1].end(), 0.0f);
   std::fill(mRevBuf1[0].begin(), mRevBuf1[0].end(), 0.0f);
   std::fill(mRevBuf1[1].begin(), mRevBuf1[1].end(), 0.0f);
   std::fill(mRevBuf2[0].begin(), mRevBuf2[0].end(), 0.0f);
   std::fill(mRevBuf2[1].begin(), mRevBuf2[1].end(), 0.0f);
   std::fill(mRevBuf3[0].begin(), mRevBuf3[0].end(), 0.0f);
   std::fill(mRevBuf3[1].begin(), mRevBuf3[1].end(), 0.0f);
   std::fill(mStutterBuf[0].begin(), mStutterBuf[0].end(), 0.0f);
   std::fill(mStutterBuf[1].begin(), mStutterBuf[1].end(), 0.0f);
   std::fill(mRepeaterBuf[0].begin(), mRepeaterBuf[0].end(), 0.0f);
   std::fill(mRepeaterBuf[1].begin(), mRepeaterBuf[1].end(), 0.0f);
   std::fill(mReverseBuf[0].begin(), mReverseBuf[0].end(), 0.0f);
   std::fill(mReverseBuf[1].begin(), mReverseBuf[1].end(), 0.0f);
   mReverseReadOffset = 0;
}

void DiceSynth::RollDice()
{
   if (!mHasDroppedSample || mSourceSample.LengthInSamples() <= 0)
      return; //nothing to mold until a sample is dropped in

   mLastRoll = 1 + (int)ofRandom(5.999f);

   if (mMoldMode == kMold_Effects)
      RollDiceEffects();
   else
      RollDiceResynth();

   mPlayPos = 0;
   ResetLiveEffectState();
}

void DiceSynth::IterateMold()
{
   if (!mHasMolded || mMoldedLength <= 0)
      return;

   //the molded output becomes the new source
   const float* molded = mMoldedSample.Data()->GetChannel(0);
   std::vector<float> fed(mMoldedLength);
   for (int i = 0; i < mMoldedLength; ++i)
      fed[i] = molded[i];

   mSourceSample.Create(mMoldedLength);
   float* dst = mSourceSample.Data()->GetChannel(0);
   for (int i = 0; i < mMoldedLength; ++i)
      dst[i] = fed[i];

   ++mGeneration;
   mSourceSample.SetName("gen " + ofToString(mGeneration));

   //the pitch shift is now baked into the audio we just fed back, so re-applying it every generation
   //would compound multiplicatively and run straight out of the audible range
   mGenome.pitchShiftSemitones = 0.0f;

   if (mMoldMode == kMold_Resynth)
   {
      //DETERMINISTIC: re-analyse the fed-back audio but hold the genome fixed. Nothing random happens
      //here - the change comes purely from the model re-reading its own output, which is what makes
      //the synthesis engine's character accumulate instead of averaging out.
      AnalyzeSample();
      mNeedsReanalysis = false;
      if (mAnalysis.valid)
         RenderFromGenome();
   }
   else
   {
      mNeedsReanalysis = true;
      RollDiceEffects(); //fresh effects each generation - compounds like tape delay feedback
   }

   mPlayPos = 0;
   ResetLiveEffectState();
}

void DiceSynth::ResetToOriginal()
{
   if (mOriginalSample.LengthInSamples() <= 0)
      return;

   mSourceSample.CopyFrom(&mOriginalSample);
   mGeneration = 0;
   mNeedsReanalysis = true;
   mRegionStart = 0.0f;
   mRegionEnd = 1.0f;

   if (mMoldMode == kMold_Resynth)
   {
      AnalyzeSample();
      mNeedsReanalysis = false;
      if (mAnalysis.valid)
      {
         InitGenomeFromAnalysis();
         mGenomeValid = true;
         RenderFromGenome();
      }
   }
   else
   {
      RollDiceEffects();
   }

   mPlayPos = 0;
   ResetLiveEffectState();
}

void DiceSynth::RollDiceEffects()
{
   //effects mode bakes straight DSP with no FFT analysis, so it can afford the whole sample - the
   //mold-length cap exists only to keep resynth's analysis affordable, and is hidden in this mode.
   //Capping here as well was silently truncating a dropped song to the slider's value.
   int len = MoldedCapacity();
   if (len <= 0)
      return;

   std::vector<float> buf(len);
   const float* src = mSourceSample.Data()->GetChannel(0);
   for (int i = 0; i < len; ++i)
      buf[i] = src[i];

   const float c = ofClamp(mChaos, 0.0f, 1.0f);

   //keep the untouched sample so low chaos can blend back toward it - the most direct way to make
   //"less chaotic" actually mean "closer to the original" rather than just "milder effects"
   const float wet = ofLerp(0.3f, 1.0f, c);
   std::vector<float> dry;
   if (wet < 0.99f)
      dry = buf;

   //the die's face value is how MANY transformations stack; chaos scales that down, so a 6 at low
   //chaos is a couple of gentle moves rather than a pile-up
   int effectCount = MAX(1, (int)ceilf(mLastRoll * ofLerp(0.25f, 1.0f, c)));

   bool used[kNumLiveEffects] = { };
   std::vector<int> order;
   int guard = 0;
   while ((int)order.size() < effectCount && guard++ < 200)
   {
      int idx = (int)ofRandom((float)kNumLiveEffects - 0.001f);
      //reverse has no mild setting - it is all or nothing - so keep it out of the pool until chaos
      //is high enough to actually want that move
      if (idx == kFxLive_Reverse && c < 0.35f)
         continue;
      if (!used[idx])
      {
         used[idx] = true;
         order.push_back(idx);
      }
   }

   static const char* kNames[kNumLiveEffects] = { "filter", "comb", "chorus", "pitch", "shift", "delay", "verb", "stutter", "repeat", "rev" };
   mMoldDescription.clear();

   for (int idx : order)
   {
      switch (idx)
      {
         //every range's endpoints are interpolated by chaos: at 0 these are barely-there settings,
         //at 1 they are the extremes. Note some parameters get milder as they go UP (filter cutoff,
         //bitcrush bit depth, stutter/repeat length), so those interpolate downward.
         case kFxLive_Filter:
            BakeFilter(buf, ofRandom(ofLerp(3000.0f, 150.0f, c), ofLerp(14000.0f, 2500.0f, c)));
            break;
         case kFxLive_Comb:
            BakeComb(buf, ofRandom(80.0f, 900.0f), ofRandom(ofLerp(0.3f, 0.6f, c), ofLerp(0.6f, 0.92f, c)));
            break;
         case kFxLive_Chorus:
            BakeChorus(buf, ofRandom(0.15f, 2.5f), ofRandom(ofLerp(0.6f, 3.0f, c), ofLerp(2.5f, 14.0f, c)));
            break;
         case kFxLive_Pitch:
            BakePitchShift(buf, ofRandom(-1.0f, 1.0f) * ofLerp(1.0f, 12.0f, c));
            break;
         case kFxLive_FreqShift:
            BakeFreqShift(buf, ofRandom(-1.0f, 1.0f) * ofLerp(15.0f, 400.0f, c));
            break;
         case kFxLive_Delay:
            BakeDelay(buf, (int)(gSampleRate * ofRandom(0.02f, 0.5f)), ofRandom(ofLerp(0.1f, 0.3f, c), ofLerp(0.3f, 0.85f, c)));
            break;
         case kFxLive_Reverb:
            BakeReverb(buf, ofRandom(ofLerp(0.2f, 0.4f, c), ofLerp(0.45f, 0.9f, c)));
            break;
         case kFxLive_Stutter:
            BakeStutter(buf, (int)(gSampleRate * ofRandom(ofLerp(0.15f, 0.02f, c), ofLerp(0.4f, 0.2f, c))));
            break;
         case kFxLive_Repeater:
            BakeRepeater(buf, (int)(gSampleRate * ofRandom(ofLerp(0.5f, 0.1f, c), ofLerp(1.2f, 0.7f, c))));
            break;
         case kFxLive_Reverse:
            BakeReverse(buf);
            break;
      }
      if (!mMoldDescription.empty())
         mMoldDescription += "+";
      mMoldDescription += kNames[idx];
   }

   if (!dry.empty())
   {
      for (int i = 0; i < len; ++i)
         buf[i] = dry[i] * (1.0f - wet) + buf[i] * wet;
   }

   WriteMolded(buf, "roll #" + ofToString(mLastRoll));
}

void DiceSynth::RollDiceResynth()
{
   if (mNeedsReanalysis)
   {
      AnalyzeSample();
      if (!mGenomeValid) //a genuinely new sample - a loaded/restored genome should NOT be reset here
      {
         InitGenomeFromAnalysis();
         mGenomeValid = true;
      }
      mNeedsReanalysis = false;
   }

   if (!mAnalysis.valid)
      return;

   mMoldDescription = "tonal " + ofToString((int)(mAnalysis.harmonicity * 100)) + "%  " + ofToString((int)mAnalysis.f0) + "Hz";
   MutateGenome(mLastRoll);
   RenderFromGenome();
}

void DiceSynth::WriteMolded(const std::vector<float>& buf, const std::string& name)
{
   int len = (int)buf.size();
   if (len <= 0)
      return;

   EnsureMoldedCapacity();
   //never write past the allocation - the capacity is fixed for the life of this dropped sample
   len = MIN(len, mMoldedSample.LengthInSamples());
   if (len <= 0)
      return;

   float peak = 0.0f;
   for (int i = 0; i < len; ++i)
      peak = MAX(peak, fabsf(buf[i]));
   float norm = (peak > 1e-6f) ? (0.9f / peak) : 1.0f;

   float* dst = mMoldedSample.Data()->GetChannel(0);
   for (int i = 0; i < len; ++i)
      dst[i] = buf[i] * norm;
   //silence whatever is left of the over-allocated buffer so a shorter mold can't play back the
   //tail of a previous, longer one
   for (int i = len; i < mMoldedSample.LengthInSamples(); ++i)
      dst[i] = 0.0f;

   mMoldedSample.SetName(name);
   mMoldedLength = len;
   mHasMolded = true;
}

int DiceSynth::MoldedCapacity() const
{
   //keyed to the ORIGINAL drop, never the working source: iteration swaps in a shorter source, and
   //if capacity followed that it would shrink and force a realloc of the very buffer the audio
   //thread is reading - reintroducing the use-after-free that was fixed earlier.
   int sourceLen = mOriginalSample.LengthInSamples();
   return MIN(sourceLen, (int)(300.0f * gSampleRate));
}

void DiceSynth::EnsureMoldedCapacity()
{
   int capacity = MoldedCapacity();
   if (capacity <= 0)
      return;

   if (mMoldedSample.LengthInSamples() != capacity)
   {
      //this is the only place the buffer is ever (re)allocated, and it happens on sample load
      //rather than on a dice roll or a slider drag, so the audio thread is not mid-playback of it
      mHasMolded = false;
      mMoldedLength = 0;
      mMoldedSample.Create(capacity);
   }
}

void DiceSynth::BakeFilter(std::vector<float>& buf, float cutoffHz)
{
   float f = 2.0f * sinf((float)PI * cutoffHz / gSampleRate);
   float low = 0.0f, band = 0.0f;
   for (size_t i = 0; i < buf.size(); ++i)
   {
      low += f * band;
      float high = buf[i] - low - 0.5f * band;
      band += f * high;
      buf[i] = low;
   }
}

void DiceSynth::BakeComb(std::vector<float>& buf, float pitchHz, float feedback)
{
   //a feedback delay whose length is one period of pitchHz, so it rings at that pitch - imposes a
   //tuned metallic resonance on whatever goes through it
   int delay = (int)(gSampleRate / MAX(20.0f, pitchHz));
   if (delay < 2 || delay >= (int)buf.size())
      return;

   std::vector<float> line(delay, 0.0f);
   int pos = 0;
   float fb = ofClamp(feedback, 0.0f, 0.95f);
   for (size_t i = 0; i < buf.size(); ++i)
   {
      float delayed = line[pos];
      float out = buf[i] + delayed * fb;
      line[pos] = out;
      buf[i] = out * (1.0f - fb * 0.5f); //trim as resonance rises so it doesn't run away in level
      pos = (pos + 1) % delay;
   }
}

void DiceSynth::BakeChorus(std::vector<float>& buf, float rateHz, float depthMs)
{
   //a short delay swept by an LFO. Mixed against the dry signal the moving delay produces the
   //detuned shimmer that reads as width and motion.
   int n = (int)buf.size();
   int maxDelay = (int)(gSampleRate * 0.03f);
   if (n < maxDelay * 2 || maxDelay < 4)
      return;

   std::vector<float> line(maxDelay, 0.0f);
   int pos = 0;
   float phase = 0.0f;
   float phaseInc = rateHz / gSampleRate;
   float baseDelay = gSampleRate * 0.012f;
   float depth = MIN(gSampleRate * depthMs * 0.001f, baseDelay - 2.0f);

   std::vector<float> out(n);
   for (int i = 0; i < n; ++i)
   {
      line[pos] = buf[i];
      float d = baseDelay + depth * sinf(phase * (float)TWO_PI);
      d = ofClamp(d, 1.0f, (float)(maxDelay - 2));
      float readPos = (float)pos - d;
      while (readPos < 0)
         readPos += maxDelay;
      int r0 = (int)readPos % maxDelay;
      int r1 = (r0 + 1) % maxDelay;
      float f = readPos - floorf(readPos);
      float wet = line[r0] * (1.0f - f) + line[r1] * f;
      out[i] = buf[i] * 0.6f + wet * 0.6f;
      pos = (pos + 1) % maxDelay;
      phase += phaseInc;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
   buf = out;
}

void DiceSynth::BakePitchShift(std::vector<float>& buf, float semitones)
{
   //read through at a shifted rate and wrap, so pitch changes but the length doesn't. Cubic rather
   //than linear interpolation here - a linear read is close enough to a zero-order hold to add its
   //own gritty aliasing on top of whatever the dice actually asked for.
   int n = (int)buf.size();
   if (n < 4)
      return;

   float rate = powf(2.0f, semitones / 12.0f);
   std::vector<float> out(n);
   double readPos = 0.0;
   for (int i = 0; i < n; ++i)
   {
      out[i] = CubicInterp(buf.data(), n, readPos);
      readPos += rate;
      while (readPos >= n)
         readPos -= n;
   }
   buf = out;
}

void DiceSynth::BakeFreqShift(std::vector<float>& buf, float shiftHz)
{
   float phase = 0.0f;
   float phaseInc = (float)TWO_PI * shiftHz / gSampleRate;
   for (size_t i = 0; i < buf.size(); ++i)
   {
      buf[i] *= cosf(phase);
      phase += phaseInc;
      if (phase > (float)TWO_PI)
         phase -= (float)TWO_PI;
      else if (phase < -(float)TWO_PI)
         phase += (float)TWO_PI;
   }
}

void DiceSynth::BakeDelay(std::vector<float>& buf, int delaySamples, float feedback)
{
   int n = (int)buf.size();
   if (delaySamples < 1 || delaySamples >= n)
      return;

   std::vector<float> line(delaySamples, 0.0f);
   int writePos = 0;
   for (int i = 0; i < n; ++i)
   {
      float delayed = line[writePos];
      float input = buf[i];
      line[writePos] = input + delayed * feedback;
      buf[i] = input * 0.6f + delayed * 0.6f;
      writePos = (writePos + 1) % delaySamples;
   }
}

void DiceSynth::BakeReverb(std::vector<float>& buf, float feedback)
{
   int n = (int)buf.size();
   std::vector<float> comb1(kRevDel1, 0.0f), comb2(kRevDel2, 0.0f), comb3(kRevDel3, 0.0f);
   int i1 = 0, i2 = 0, i3 = 0;
   for (int i = 0; i < n; ++i)
   {
      comb1[i1] = buf[i] + comb1[i1] * feedback;
      comb2[i2] = buf[i] + comb2[i2] * feedback;
      comb3[i3] = buf[i] + comb3[i3] * feedback;
      float wet = (comb1[i1] + comb2[i2] + comb3[i3]) * 0.33f;
      buf[i] = buf[i] * 0.5f + wet * 0.5f;
      i1 = (i1 + 1) % kRevDel1;
      i2 = (i2 + 1) % kRevDel2;
      i3 = (i3 + 1) % kRevDel3;
   }
}

void DiceSynth::BakeStutter(std::vector<float>& buf, int stutterLenSamples)
{
   int n = (int)buf.size();
   if (stutterLenSamples < 32 || stutterLenSamples * 2 >= n)
      return;

   int period = stutterLenSamples * 4;
   for (int start = period; start + stutterLenSamples <= n; start += period)
   {
      for (int rep = 0; rep < 3 && start + stutterLenSamples <= n; ++rep)
      {
         for (int i = 0; i < stutterLenSamples; ++i)
            buf[start + i] = buf[start - stutterLenSamples + i];
      }
   }
}

void DiceSynth::BakeRepeater(std::vector<float>& buf, int segmentLenSamples)
{
   int n = (int)buf.size();
   if (segmentLenSamples < 256 || segmentLenSamples >= n)
      return;

   int segmentStart = (int)ofRandom((float)(n - segmentLenSamples));
   for (int i = segmentStart + segmentLenSamples; i < n; ++i)
      buf[i] = buf[segmentStart + ((i - segmentStart) % segmentLenSamples)];
}

void DiceSynth::BakeReverse(std::vector<float>& buf)
{
   std::reverse(buf.begin(), buf.end());
}

float DiceSynth::EstimatePitch(const float* data, int len, float sampleRate, float* outConfidence) const
{
   if (outConfidence != nullptr)
      *outConfidence = 0.0f;

   //YIN's cumulative-mean-normalized difference function, rather than plain autocorrelation.
   //Plain autocorrelation is maximal at the SHORTEST lag for any bass-heavy signal - 29 samples
   //apart, a 50Hz waveform has barely moved, so it correlates ~0.9 with itself - which made the
   //old detector slide straight to its own minimum lag and report sampleRate/minLag (1520Hz) for
   //every kick. YIN's normalization removes exactly that bias, and its dip depth doubles as a
   //trustworthy periodicity confidence.
   int minLag = (int)(sampleRate / 1500.0f);
   int maxLag = MIN((int)(sampleRate / 40.0f), len / 2);
   if (maxLag <= minLag + 1)
      return 110.0f;

   int window = len - maxLag;
   if (window < 128)
      return 110.0f;

   std::vector<float> diff(maxLag + 1, 0.0f);
   for (int lag = minLag; lag <= maxLag; ++lag)
   {
      float sum = 0.0f;
      for (int i = 0; i < window; ++i)
      {
         float d = data[i] - data[i + lag];
         sum += d * d;
      }
      diff[lag] = sum;
   }

   //cumulative mean normalization: d'(lag) starts near 1 and only dips where the signal genuinely
   //repeats, so short lags no longer win by default
   std::vector<float> norm(maxLag + 1, 1.0f);
   double running = 0.0;
   for (int lag = minLag; lag <= maxLag; ++lag)
   {
      running += diff[lag];
      int count = lag - minLag + 1;
      norm[lag] = (running > 1e-12) ? (float)(diff[lag] * count / running) : 1.0f;
   }

   //take the FIRST clear dip rather than the global minimum - that's what avoids octave errors
   const float kYinThreshold = 0.15f;
   int bestLag = -1;
   for (int lag = minLag + 1; lag < maxLag; ++lag)
   {
      if (norm[lag] < kYinThreshold && norm[lag] <= norm[lag + 1])
      {
         bestLag = lag;
         break;
      }
   }
   if (bestLag < 0)
   {
      float lowest = 1e9f;
      for (int lag = minLag; lag <= maxLag; ++lag)
      {
         if (norm[lag] < lowest)
         {
            lowest = norm[lag];
            bestLag = lag;
         }
      }
   }
   if (bestLag <= 0)
      return 110.0f;

   //octave correction. YIN's first-dip rule can latch onto a HARMONIC rather than the fundamental,
   //reporting a pitch an octave (or twelfth) too high. That is not a small error here: a harmonic
   //series built on 2*f0 only lines up with every OTHER real partial, so the salience gate below
   //sees half its measurement windows land in gaps and collapses - which is what makes otherwise
   //identical guitar samples read 40% or 10% at random. Prefer a longer lag when it dips about as
   //deep, since the true fundamental is the longest lag that still explains the signal.
   for (int multiple = 2; multiple <= 3; ++multiple)
   {
      int candidate = bestLag * multiple;
      if (candidate > maxLag)
         break;
      if (norm[candidate] < norm[bestLag] * 1.15f)
         bestLag = candidate;
   }

   if (outConfidence != nullptr)
      *outConfidence = ofClamp(1.0f - norm[bestLag], 0.0f, 1.0f);

   return sampleRate / (float)bestLag;
}

float DiceSynth::ComputeVoicing(const float* data, int totalLen, int start, int frameLen, float f0) const
{
   int lag = (int)(gSampleRate / MAX(20.0f, f0));
   if (lag < 2)
      return 0.0f;

   //clip the comparison to what's actually in the buffer
   int n = frameLen;
   if (start < 0)
      return 0.0f;
   if (start + n + lag > totalLen)
      n = totalLen - start - lag;
   if (n < 64)
      return 0.0f;

   double num = 0.0, energyA = 0.0, energyB = 0.0;
   for (int i = 0; i < n; ++i)
   {
      float a = data[start + i];
      float b = data[start + i + lag];
      num += (double)a * b;
      energyA += (double)a * a;
      energyB += (double)b * b;
   }

   //silence gate: autocorrelation happily reports a confident-looking pitch on near-nothing
   if (energyA / n < 1e-8 || energyB / n < 1e-8)
      return 0.0f;

   double den = sqrt(energyA * energyB);
   if (den < 1e-12)
      return 0.0f;
   return (float)ofClamp(num / den, 0.0, 1.0);
}

void DiceSynth::AnalyzeSample()
{
   mAnalysis = SampleAnalysis();

   int len = MoldedCapacity();
   if (len <= 0)
      return;

   const float* data = mSourceSample.Data()->GetChannel(0);

   //---- pitch: autocorrelate a representative window taken from the loudest part of the sample ----
   int pitchWindowSize = MIN(len, 8192);
   int bestStart = 0;
   float bestEnergy = -1.0f;
   int scanStep = MAX(1, (len - pitchWindowSize) / 32);
   for (int start = 0; start + pitchWindowSize <= len; start += scanStep)
   {
      float energy = 0.0f;
      for (int i = 0; i < pitchWindowSize; i += 4) //subsampled scan, just picking the loudest region
         energy += data[start + i] * data[start + i];
      if (energy > bestEnergy)
      {
         bestEnergy = energy;
         bestStart = start;
      }
   }
   float globalConfidence = 0.0f;
   mAnalysis.f0 = ofClamp(EstimatePitch(data + bestStart, pitchWindowSize, (float)gSampleRate, &globalConfidence), 40.0f, 2000.0f);
   mAnalysis.globalVoiced = globalConfidence;

   //hard cutoff for material with no real periodicity at all (hi-hat, clap, cymbal, field
   //recording): don't let the harmonic path contribute anything, however confident a single frame
   //might look. Below this the residual simply IS the sample, which is the correct answer for noise.
   const float kUnpitchedFloor = 0.25f;
   bool treatAsUnpitched = (globalConfidence < kUnpitchedFloor);

   //---- frame-by-frame spectral analysis: track each harmonic partial's amplitude over time, plus
   //---- whatever energy isn't accounted for by the harmonics (the "noise" residual) ----
   const int fftSize = kAnalysisFrameSize;
   FFT fft(fftSize);
   std::vector<float> window(fftSize);
   for (int i = 0; i < fftSize; ++i)
      window[i] = 0.5f - 0.5f * cosf((float)TWO_PI * i / (fftSize - 1)); //Hann

   std::vector<float> fftInput(fftSize);
   int numBins = fftSize / 2 + 1;
   std::vector<float> fftReal(numBins);
   std::vector<float> fftImag(numBins);

   //a 256-sample hop is worth it on a one-shot, where a few milliseconds of attack detail decide
   //whether a kick reads as a kick. On a multi-minute song it just multiplies the frame count (and
   //the FFT cost) for detail nobody can hear, so step up the hop once the material gets long.
   int hop = kAnalysisHop;
   if (len > (int)(gSampleRate * 20))
      hop = kAnalysisHop * 4;
   else if (len > (int)(gSampleRate * 5))
      hop = kAnalysisHop * 2;

   //frames are CENTERED on frame*hop and run past both ends of the buffer, so the overlap-add
   //below covers sample 0 fully - otherwise the attack (exactly where a kick lives) would be
   //reconstructed at partial amplitude
   int numFrames = (len + fftSize) / hop + 1;
   mAnalysis.numFrames = numFrames;
   mAnalysis.frameDurationSec = (float)hop / gSampleRate;
   for (int h = 0; h < kNumHarmonics; ++h)
      mAnalysis.harmonicEnvelope[h].assign(numFrames, 0.0f);
   mAnalysis.f0Envelope.assign(numFrames, mAnalysis.f0);
   mAnalysis.voicing.assign(numFrames, 0.0f);
   mAnalysis.residual.assign(len, 0.0f);

   float binHz = (float)gSampleRate / fftSize;
   float bestFrameEnergy = -1.0f;

   std::vector<float> mag(numBins);
   std::vector<bool> claimed(numBins, false);
   std::vector<float> ifftOut(fftSize);
   float harmonicityWeightedSum = 0.0f;
   float harmonicityWeight = 0.0f;
   float trackedF0 = mAnalysis.f0;

   for (int frame = 0; frame < numFrames; ++frame)
   {
      int start = frame * hop - fftSize / 2;
      for (int i = 0; i < fftSize; ++i)
      {
         int idx = start + i;
         fftInput[i] = (idx >= 0 && idx < len ? data[idx] : 0.0f) * window[i];
      }
      fft.Forward(fftInput.data(), fftReal.data(), fftImag.data());

      float totalEnergy = 0.0f;
      float frameMaxMag = 0.0f;
      for (int b = 0; b < numBins; ++b)
      {
         mag[b] = sqrtf(fftReal[b] * fftReal[b] + fftImag[b] * fftImag[b]);
         totalEnergy += mag[b] * mag[b];
         frameMaxMag = MAX(frameMaxMag, mag[b]);
      }

      //---- pitch tracking: follow the fundamental frame to frame, anchored near where it was last
      //---- frame. A kick/tom sweeps downward as it decays; holding one fixed f0 for the whole
      //---- sample put every harmonic in the wrong place.
      if (frameMaxMag > 1e-5f)
      {
         int loBin = MAX(1, (int)(trackedF0 * 0.55f / binHz));
         int hiBin = MIN(numBins - 2, (int)(trackedF0 * 1.8f / binHz));
         int bestBin = -1;
         float bestMag = 0.0f;
         for (int b = loBin; b <= hiBin; ++b)
         {
            if (mag[b] > bestMag)
            {
               bestMag = mag[b];
               bestBin = b;
            }
         }
         if (bestBin > 0 && bestMag > 0.1f * frameMaxMag) //ignore weak/absent fundamentals rather than chasing noise
         {
            float refined = ParabolicPeakBin(mag.data(), bestBin, numBins) * binHz;
            //ease toward the new estimate so the track glides instead of jumping on a bad frame,
            //and keep it within an octave of the global estimate - a fundamental has no business
            //wandering several octaves, and letting it climb is how noisy material ends up tracked
            //to a high "pitch" where short-lag correlation looks deceptively strong
            float glided = trackedF0 * 0.5f + refined * 0.5f;
            trackedF0 = ofClamp(glided, MAX(30.0f, mAnalysis.f0 * 0.5f), MIN(4000.0f, mAnalysis.f0 * 2.0f));
         }
      }
      mAnalysis.f0Envelope[frame] = trackedF0;

      //---- voicing confidence decides how much of this frame the harmonic path may claim. On a hat
      //---- or a clap this lands near zero, so no fake sines get built on a meaningless pitch, and
      //---- the residual below keeps the whole spectrum instead of a comb-notched version of it.
      float voicing = treatAsUnpitched ? 0.0f : ComputeVoicing(data, len, MAX(0, start), fftSize, trackedF0);

      //---- harmonic salience gate. Periodicity alone doesn't mean "pitched": a cymbal's metallic
      //---- resonances repeat, so autocorrelation reports 30%+ on a hi-hat with no fundamental at
      //---- all. This asks the question that actually matters - does a harmonic series at trackedF0
      //---- explain this spectrum better than chance? - by comparing energy ON the harmonics against
      //---- energy in the gaps BETWEEN them. Both windows sit adjacent in frequency, so the sample's
      //---- overall spectral tilt affects them equally and cancels; that is precisely what a
      //---- wideband flatness measure gets wrong, since a hi-hat's steep slope reads as "peaky".
      {
         int radius = (int)ofClamp(trackedF0 / binHz * 0.4f, 1.0f, 8.0f);
         double onHarmonic = 0.0, betweenHarmonics = 0.0;
         for (int h = 0; h < kNumHarmonics; ++h)
         {
            float freq = trackedF0 * (h + 1);
            if (freq >= gSampleRate * 0.5f)
               break;

            int centerBin = (int)(freq / binHz + 0.5f);
            for (int b = MAX(0, centerBin - radius); b <= MIN(numBins - 1, centerBin + radius); ++b)
               onHarmonic += (double)mag[b] * mag[b];

            //the midpoint to the next harmonic - where a real harmonic series has a gap
            float gapFreq = trackedF0 * (h + 1.5f);
            if (gapFreq < gSampleRate * 0.5f)
            {
               int gapBin = (int)(gapFreq / binHz + 0.5f);
               for (int b = MAX(0, gapBin - radius); b <= MIN(numBins - 1, gapBin + radius); ++b)
                  betweenHarmonics += (double)mag[b] * mag[b];
            }
         }

         double totalCompared = onHarmonic + betweenHarmonics;
         //0.5 means the harmonics are no better populated than the gaps, i.e. noise
         float salience = (totalCompared > 1e-12) ? (float)(onHarmonic / totalCompared) : 0.5f;
         float salience01 = ofClamp((salience - 0.5f) * 2.0f, 0.0f, 1.0f);

         //gate hard below 0.05 (noise), pass freely above 0.25. Cymbals sit essentially at zero, so
         //a low floor still kills them, while a kick's partially-harmonic body keeps its level -
         //the earlier 0.12/0.40 window was squeezing kicks down along with the hats.
         float t = ofClamp((salience01 - 0.05f) / 0.20f, 0.0f, 1.0f);
         voicing *= t * t * (3.0f - 2.0f * t); //smoothstep
      }

      mAnalysis.voicing[frame] = voicing;

      //---- harmonics at this frame's tracked pitch, scaled by voicing ----
      std::fill(claimed.begin(), claimed.end(), false);
      for (int h = 0; h < kNumHarmonics; ++h)
      {
         float freq = trackedF0 * (h + 1);
         if (freq >= gSampleRate * 0.5f)
            break; //above Nyquist there's nothing meaningful to measure

         int centerBin = (int)(freq / binHz + 0.5f);
         //capture width tracks the harmonic SPACING rather than a fixed +/-2 bins. At f0=50Hz a
         //fixed +/-2 bins is +/-43Hz against 50Hz spacing, so neighbouring harmonics overlapped and
         //every partial read a similar bogus amplitude - which is why bass and kicks came out wrong.
         int searchRadius = (int)ofClamp(trackedF0 / binHz * 0.4f, 1.0f, 8.0f);
         float peak = 0.0f;
         for (int b = MAX(0, centerBin - searchRadius); b <= MIN(numBins - 1, centerBin + searchRadius); ++b)
         {
            peak = MAX(peak, mag[b]);
            claimed[b] = true;
         }
         mAnalysis.harmonicEnvelope[h][frame] = peak * voicing;
         mAnalysis.harmonicPeak[h] = MAX(mAnalysis.harmonicPeak[h], mAnalysis.harmonicEnvelope[h][frame]);
      }

      //---- residual: keep the ACTUAL waveform. Attenuate only the bins the harmonic path claimed
      //---- (by the same voicing amount, so energy is conserved rather than notched out), leave the
      //---- original PHASES untouched, then inverse-FFT and overlap-add. This is what makes a click
      //---- come back as a click - synthesizing it from band amplitudes never could.
      if (voicing > 1e-4f)
      {
         for (int b = 0; b < numBins; ++b)
         {
            if (claimed[b])
            {
               float keep = 1.0f - voicing;
               fftReal[b] *= keep;
               fftImag[b] *= keep;
            }
         }
         fft.Inverse(fftReal.data(), fftImag.data(), ifftOut.data());
         for (int i = 0; i < fftSize; ++i)
         {
            int idx = start + i;
            if (idx >= 0 && idx < len)
               mAnalysis.residual[idx] += ifftOut[i];
         }
      }
      else
      {
         //nothing was claimed, so the residual for this frame is just the windowed input. Rebuilding
         //that via a full inverse FFT is exact but pointless - and on polyphonic material, where
         //voicing is zero for every frame, it doubles the entire analysis cost for no reason. The
         //OLA scale below is divided by fftSize/2 to match what the inverse transform would apply.
         const float ifftGain = (float)fftSize * 0.5f;
         for (int i = 0; i < fftSize; ++i)
         {
            int idx = start + i;
            if (idx >= 0 && idx < len)
               mAnalysis.residual[idx] += fftInput[i] * ifftGain;
         }
      }

      if (totalEnergy > 1e-9f)
      {
         harmonicityWeightedSum += voicing * totalEnergy;
         harmonicityWeight += totalEnergy;
      }

      if (totalEnergy > bestFrameEnergy)
      {
         bestFrameEnergy = totalEnergy;
         mAnalysis.peakFrame = frame;
      }
   }

   //undo the inverse-FFT scaling (N/2) and the overlap-add window sum (N/hop * 0.5 for Hann)
   const float olaScale = 4.0f * hop / ((float)fftSize * (float)fftSize);
   for (auto& v : mAnalysis.residual)
      v *= olaScale;

   mAnalysis.harmonicity = (harmonicityWeight > 1e-9f) ? ofClamp(harmonicityWeightedSum / harmonicityWeight, 0.0f, 1.0f) : 0.0f;

   //scale the harmonic coefficients so the additive stack comes out at roughly the source's own
   //level (the residual is already at its natural level, being the real signal - don't touch it)
   //FFT magnitude -> oscillator amplitude is a FIXED conversion: a Hann-windowed sine of amplitude A
   //reads as A*N/4, so the inverse is 4/N. The previous code instead normalized so the loudest
   //single partial hit half the source peak - which is wrong in two ways. It divided by a
   //near-zero harmonic peak on barely-tonal material (blowing the harmonic path up by a factor of
   //thousands), and it ignored that 48 partials SUM, so the additive stack drowned the residual and
   //the final peak-normalize then crushed the real audio away. That is why every genome converged
   //to the same synthetic timbre regardless of the source.
   const float harmScale = 4.0f / (float)fftSize;
   for (int h = 0; h < kNumHarmonics; ++h)
      for (auto& v : mAnalysis.harmonicEnvelope[h])
         v *= harmScale;
   for (int h = 0; h < kNumHarmonics; ++h)
      mAnalysis.harmonicPeak[h] *= harmScale; //keep the peaks in the same units as the envelopes

   mAnalysis.valid = true;
}

void DiceSynth::InitGenomeFromAnalysis()
{
   for (int h = 0; h < kNumHarmonics; ++h)
      mGenome.harmonicAmp[h] = 1.0f; //baseline: reproduce the analyzed amount as-is, no mutation yet
   for (int b = 0; b < kNumNoiseBands; ++b)
      mGenome.noiseBandAmp[b] = 1.0f;

   //the residual is the real leftover signal, so its natural level is simply 1.0 - no ratio to
   //measure. At this baseline the harmonics plus the residual reconstruct the original sample.
   mGenome.noiseAmount = 1.0f;
   mGenome.baselineNoiseAmount = 1.0f;

   mGenome.attackScale = 1.0f;
   mGenome.decayScale = 1.0f;
   mGenome.brightnessTilt = 0.0f;
   mGenome.inharmonicity = 0.0f;
   mGenome.pitchShiftSemitones = 0.0f;
   mGenome.harmonicStretch = 1.0f;
   mGenome.reverseResidual = false;
}

void DiceSynth::MutateGenome(int rollStrength)
{
   //the die's face value sets how big a jump this generation takes - low rolls are subtle nudges,
   //high rolls are wild jumps, so re-rolling becomes a way to dial in how far to wander.
   //independent per-harmonic jitter alone tends to average out perceptually (some partials go up,
   //some go down, the overall spectral shape barely moves) - the structural moves below (a real
   //pitch shift, and decimating which harmonics survive at all) are what actually make each roll
   //sound like a clearly different sound rather than a slightly-different-sounding one.
   float strength = ofLerp(0.35f, 1.4f, ofClamp((rollStrength - 1) / 5.0f, 0.0f, 1.0f));
   //chaos scales the whole mutation. Every structural move below (decimation, shuffle, dropouts,
   //residual reverse) has its probability multiplied by strength too, so this one factor makes low
   //chaos mean small nudges around the analysed sound and high chaos mean genuine departures.
   strength *= ofLerp(0.25f, 1.3f, ofClamp(mChaos, 0.0f, 1.0f));

   //gentle reversion toward the analyzed baseline every generation, applied BEFORE this roll's
   //jitter. Without this, a harmonic knocked to zero (or noiseAmount drifting down) can never
   //recover - repeated rolls become a one-way ratchet toward silence instead of continuing to
   //explore different-but-still-audible variations.
   const float pullback = 0.1f;

   //structural: occasionally keep only every Nth harmonic - a drastic, obviously audible timbral
   //change (hollow/clarinet-like or metallic/bell-like), not just a volume wobble
   if (ofRandom(1.0f) < strength * 0.3f)
   {
      int keepEvery = 2 + (int)ofRandom(5.0f); //2..6 - higher values get very hollow
      int phase = (int)ofRandom((float)keepEvery);
      for (int h = 0; h < kNumHarmonics; ++h)
      {
         if ((h + phase) % keepEvery != 0)
            mGenome.harmonicAmp[h] *= 0.05f;
      }
   }

   //structural: shuffle amplitudes between partials. Moves energy to a different part of the
   //spectrum without changing how much there is, which reads as a formant/vowel shift - a kind of
   //change no amount of per-partial jitter produces on its own.
   if (ofRandom(1.0f) < strength * 0.25f)
   {
      int swaps = 1 + (int)(strength * 6.0f);
      for (int s = 0; s < swaps; ++s)
      {
         int a = (int)ofRandom((float)kNumHarmonics - 0.001f);
         int b = (int)ofRandom((float)kNumHarmonics - 0.001f);
         std::swap(mGenome.harmonicAmp[a], mGenome.harmonicAmp[b]);
      }
   }

   for (int h = 0; h < kNumHarmonics; ++h)
   {
      mGenome.harmonicAmp[h] = mGenome.harmonicAmp[h] * (1.0f - pullback) + pullback; //baseline is always 1.0
      mGenome.harmonicAmp[h] *= expf(ofRandom(-strength, strength)); //log-normal jitter, stays positive
      if (ofRandom(1.0f) < strength * 0.12f)
         mGenome.harmonicAmp[h] = 0.0f; //fully drop the partial, not just quieten it
      mGenome.harmonicAmp[h] = ofClamp(mGenome.harmonicAmp[h], 0.0f, 4.0f);
   }

   //reshape the noise spectrum too - for percussive material (kick, clap, hat) this is where most
   //of the sound actually lives, so without mutating these a drum barely changes between rolls
   for (int b = 0; b < kNumNoiseBands; ++b)
   {
      mGenome.noiseBandAmp[b] = mGenome.noiseBandAmp[b] * (1.0f - pullback) + pullback; //baseline is always 1.0
      mGenome.noiseBandAmp[b] *= expf(ofRandom(-strength, strength) * 0.8f);
      mGenome.noiseBandAmp[b] = ofClamp(mGenome.noiseBandAmp[b], 0.0f, 4.0f);
   }

   //pitch: a genuine transposition of the fundamental - the single biggest lever for making a roll
   //sound like a different sound rather than a different-flavored version of the same sound. Only
   //a little pullback here (toward the original, unshifted note) so it can wander far but doesn't
   //run away into an absurd register over dozens of rolls.
   mGenome.pitchShiftSemitones = ofClamp(mGenome.pitchShiftSemitones * (1.0f - pullback * 0.3f) + ofRandom(-strength, strength) * 10.0f, -36.0f, 36.0f);

   mGenome.noiseAmount = ofLerp(mGenome.noiseAmount, mGenome.baselineNoiseAmount, pullback);
   mGenome.noiseAmount = ofClamp(mGenome.noiseAmount + ofRandom(-strength, strength) * 0.6f, 0.0f, 2.0f);

   mGenome.attackScale = mGenome.attackScale * (1.0f - pullback) + pullback; //baseline is always 1.0
   mGenome.attackScale = ofClamp(mGenome.attackScale * expf(ofRandom(-strength, strength)), 0.15f, 6.0f);

   mGenome.decayScale = mGenome.decayScale * (1.0f - pullback) + pullback;
   mGenome.decayScale = ofClamp(mGenome.decayScale * expf(ofRandom(-strength, strength)), 0.15f, 6.0f);

   mGenome.brightnessTilt *= (1.0f - pullback); //baseline is always 0
   mGenome.brightnessTilt = ofClamp(mGenome.brightnessTilt + ofRandom(-strength, strength), -1.0f, 1.0f);

   mGenome.inharmonicity *= (1.0f - pullback);
   mGenome.inharmonicity = ofClamp(mGenome.inharmonicity + ofRandom(-strength, strength) * 0.7f, 0.0f, 1.0f);

   //partial spacing. Small numbers here are deceptive: at 1.15 the 24th partial lands where the
   //40th would, so the whole series turns bell-like rather than merely detuned.
   mGenome.harmonicStretch = mGenome.harmonicStretch * (1.0f - pullback) + pullback; //baseline is always 1.0
   mGenome.harmonicStretch = ofClamp(mGenome.harmonicStretch + ofRandom(-strength, strength) * 0.12f, 0.80f, 1.20f);

   //temporal: flip the residual. On percussion this is the reverse-cymbal move, and because it only
   //touches the residual the tonal body still plays forwards - which is far stranger than simply
   //reversing everything.
   if (ofRandom(1.0f) < strength * 0.12f)
      mGenome.reverseResidual = !mGenome.reverseResidual;
}

void DiceSynth::RenderFromGenome()
{
   if (!mAnalysis.valid)
      return;

   int len = MoldedCapacity();
   if (len <= 0)
      return;

   const float nyquistLimit = (float)gSampleRate * 0.45f;

   std::vector<float> outBuf(len, 0.0f);
   std::vector<double> phase(kNumHarmonics, 0.0);
   float peakTimeSec = mAnalysis.peakFrame * mAnalysis.frameDurationSec;
   float pitchRatio = powf(2.0f, mGenome.pitchShiftSemitones / 12.0f);

   //skip partials the genome has silenced entirely - after a few generations that's most of them,
   //and each one costs a sinf() per output sample
   //...and equally, skip partials the ANALYSIS never found anything in. Checking only the genome
   //amplitude meant polyphonic material (where voicing is 0, so every envelope is 0) still ran all
   //48 oscillators to produce pure silence - the dominant cost of rolling a full song.
   std::vector<int> activeHarmonics;
   for (int h = 0; h < kNumHarmonics; ++h)
   {
      if (mGenome.harmonicAmp[h] > 1e-4f && mAnalysis.harmonicPeak[h] > 1e-5f)
         activeHarmonics.push_back(h);
   }

   //per-partial frequency ratio and amplitude weight depend only on the genome, never on time -
   //hoist them out of the sample loop rather than paying two powf() calls per partial per sample
   std::array<float, kNumHarmonics> partialRatio{ };
   std::array<float, kNumHarmonics> partialWeight{ };
   for (int h : activeHarmonics)
   {
      float stretch = mGenome.inharmonicity * 0.12f * (float)(h * h) / (float)(kNumHarmonics * kNumHarmonics);
      partialRatio[h] = powf((float)(h + 1), mGenome.harmonicStretch) * (1.0f + stretch);
      partialWeight[h] = mGenome.harmonicAmp[h] * powf(1.0f + mGenome.brightnessTilt * 0.7f, (float)h / (float)kNumHarmonics * 8.0f - 4.0f);
   }

   //---- pitch-shift the residual. This matters most for polyphonic input (a full song, a drum
   //---- loop): there is no single fundamental, so the harmonic path is gated off and the residual
   //---- IS the whole sample - meaning without this, pitch mutation would do nothing at all on that
   //---- material. Resampling with wraparound keeps the buffer length fixed; it is done as a
   //---- pre-pass so it composes cleanly with the time warp applied in the loop below.
   const std::vector<float>* residualSource = &mAnalysis.residual;
   std::vector<float> shiftedResidual;
   if (fabsf(mGenome.pitchShiftSemitones) > 0.01f && len > 1)
   {
      shiftedResidual.resize(len);
      double readPos = 0.0;
      for (int i = 0; i < len; ++i)
      {
         shiftedResidual[i] = CubicInterp(mAnalysis.residual.data(), len, readPos);
         readPos += pitchRatio;
         while (readPos >= len)
            readPos -= len;
      }
      residualSource = &shiftedResidual;
   }

   //---- the residual's EQ. Only bands whose gain differs from 1.0 need filtering at all, and the
   //---- result is applied as an ADDITIVE correction (residual + sum of band*(gain-1)) so that an
   //---- all-1.0 genome passes the residual through bit-exact instead of through 20 overlapping
   //---- bandpasses that would ripple its spectrum.
   std::array<BiquadFilter, kNumNoiseBands> bandFilters;
   std::vector<int> activeBands;
   for (int b = 0; b < kNumNoiseBands; ++b)
   {
      if (fabsf(mGenome.noiseBandAmp[b] - 1.0f) < 1e-3f)
         continue;

      float lo = NoiseBandEdgeHz(b);
      float hi = NoiseBandEdgeHz(b + 1);
      float center = MIN(sqrtf(lo * hi), nyquistLimit);
      float q = ofClamp(center / MAX(1.0f, hi - lo), 0.5f, 8.0f);
      bandFilters[b].SetSampleRate(gSampleRate);
      bandFilters[b].SetFilterType(kFilterType_Bandpass);
      bandFilters[b].SetFilterParams(center, q);
      bandFilters[b].UpdateFilterCoeff();
      bandFilters[b].Clear();
      activeBands.push_back(b);
   }

   //---- LENGTH-PRESERVING time warp, precomputed as a small curve and interpolated per sample.
   //---- The old formula (t/attackScale, then pivot + (t-pivot)/decayScale) could run the read
   //---- position clean off the end of the sample - decayScale of 0.15 advances 6.7x faster than
   //---- output time - after which it clamped and held the final sample. On a song that tail is
   //---- silence, which is where those sudden mid-render audio dropouts came from. This mapping is
   //---- monotonic and always lands exactly on the last sample instead.
   const int kWarpPoints = 2049;
   std::vector<float> warpCurve(kWarpPoints);
   {
      float totalDur = (float)len / gSampleRate;
      float pivotSrc = ofClamp(peakTimeSec, totalDur * 0.01f, totalDur * 0.99f);
      float pivotOut = ofClamp(pivotSrc * mGenome.attackScale, totalDur * 0.02f, totalDur * 0.98f);
      for (int k = 0; k < kWarpPoints; ++k)
      {
         float t = (float)k / (kWarpPoints - 1) * totalDur;
         float warped;
         if (t <= pivotOut)
            warped = t * (pivotSrc / pivotOut); //attack: source [0,pivot] spread over output [0,pivotOut]
         else
         {
            //decay: same span both sides, but decayScale bends WHERE the time is spent inside it
            float u = (t - pivotOut) / MAX(1e-6f, totalDur - pivotOut);
            warped = pivotSrc + powf(ofClamp(u, 0.0f, 1.0f), mGenome.decayScale) * (totalDur - pivotSrc);
         }
         warpCurve[k] = warped * gSampleRate;
      }
   }

   for (int i = 0; i < len; ++i)
   {
      float curvePos = (float)i / (float)MAX(1, len - 1) * (kWarpPoints - 1);
      int c0 = (int)curvePos;
      int c1 = MIN(kWarpPoints - 1, c0 + 1);
      float cf = curvePos - c0;
      double warpedSample = warpCurve[c0] * (1.0f - cf) + warpCurve[c1] * cf;
      warpedSample = ofClamp((float)warpedSample, 0.0f, (float)(len - 1));
      float warpedT = (float)(warpedSample / gSampleRate);

      float frameF = ofClamp(warpedT / mAnalysis.frameDurationSec, 0.0f, (float)(mAnalysis.numFrames - 1));
      int f0i = (int)frameF;
      int f1i = MIN(f0i + 1, mAnalysis.numFrames - 1);
      float frac = frameF - f0i;

      //the tracked fundamental at this moment, so pitch sweeps in the source are reproduced
      float f0Now = (mAnalysis.f0Envelope[f0i] * (1.0f - frac) + mAnalysis.f0Envelope[f1i] * frac) * pitchRatio;

      float sampleVal = 0.0f;
      for (int h : activeHarmonics)
      {
         float freq = f0Now * partialRatio[h];
         if (freq >= nyquistLimit)
            continue; //ALIASING GUARD: without this, partials above Nyquist fold back down as
         //broadband garbage that sounds exactly like white noise - the single worst
         //offender for bright/transient material, and it got twice as bad when the
         //harmonic count went from 24 to 48

         float env = mAnalysis.harmonicEnvelope[h][f0i] * (1.0f - frac) + mAnalysis.harmonicEnvelope[h][f1i] * frac;

         phase[h] += (double)freq / gSampleRate;
         if (phase[h] > 1.0)
            phase[h] -= 1.0;

         sampleVal += sinf((float)(phase[h] * TWO_PI)) * env * partialWeight[h];
      }

      //the real residual waveform, read through the same time warp, then EQ'd by the genome. This
      //carries the transients: it is the original signal's own leftover, phases and all.
      double resPos = mGenome.reverseResidual ? ((double)(len - 1) - warpedSample) : warpedSample;
      float res = CubicInterp(residualSource->data(), len, resPos);
      float shaped = res;
      for (int b : activeBands)
         shaped += bandFilters[b].Filter(res) * (mGenome.noiseBandAmp[b] - 1.0f);
      sampleVal += shaped * mGenome.noiseAmount;

      outBuf[i] = sampleVal;
   }

   WriteMolded(outBuf, "genome #" + ofToString(mLastRoll));
}

float DiceSynth::ComputeEffectWeight(int effectIndex) const
{
   const float blendRadius = 0.62f;
   float dx = mXYPos[0] - mEffectSpots[effectIndex].x;
   float dy = mXYPos[1] - mEffectSpots[effectIndex].y;
   float dist = sqrtf(dx * dx + dy * dy);
   return ofClamp(1.0f - dist / blendRadius, 0.0f, 1.0f);
}

void DiceSynth::RandomizeEffects()
{
   const float c = ofClamp(mChaos, 0.0f, 1.0f);

   //at low chaos the effects stay near their even circle so the pad behaves predictably; at high
   //chaos they scatter, which makes neighbouring positions sound wildly different
   for (int e = 0; e < kNumLiveEffects; ++e)
   {
      float angle = -(float)PI * 0.5f + e * ((float)TWO_PI / kNumLiveEffects);
      float orderlyX = 0.5f + 0.42f * cosf(angle);
      float orderlyY = 0.5f + 0.42f * sinf(angle);
      mEffectSpots[e].x = ofLerp(orderlyX, ofRandom(0.12f, 0.88f), c);
      mEffectSpots[e].y = ofLerp(orderlyY, ofRandom(0.12f, 0.88f), c);
   }

   //each effect's ceiling: mild endpoint at chaos 0, extreme endpoint at chaos 1
   mFilterMinCutoffHz = ofRandom(ofLerp(1200.0f, 80.0f, c), ofLerp(3000.0f, 500.0f, c));
   mCombMaxFeedback = ofRandom(ofLerp(0.25f, 0.6f, c), ofLerp(0.55f, 0.93f, c));
   mChorusMaxDepthMs = ofRandom(ofLerp(0.5f, 3.0f, c), ofLerp(2.0f, 14.0f, c));
   mPitchMaxRateBoost = ofRandom(ofLerp(0.05f, 0.3f, c), ofLerp(0.3f, 2.0f, c));
   mFreqShiftMaxHz = ofRandom(ofLerp(5.0f, 50.0f, c), ofLerp(40.0f, 600.0f, c));
   mDelayMaxTimeMs = ofRandom(20.0f, 300.0f);
   mDelayMaxFeedback = ofRandom(ofLerp(0.1f, 0.3f, c), ofLerp(0.35f, 0.9f, c));
   mReverbMaxFeedback = ofRandom(ofLerp(0.15f, 0.3f, c), ofLerp(0.4f, 0.9f, c));
   mStutterMinLenMs = ofRandom(ofLerp(40.0f, 3.0f, c), ofLerp(120.0f, 40.0f, c));
   mRepeaterMinLenMs = ofRandom(ofLerp(150.0f, 10.0f, c), ofLerp(400.0f, 200.0f, c));
   mReverseMaxWindowMs = ofRandom(ofLerp(10.0f, 20.0f, c), ofLerp(60.0f, 300.0f, c));
}

void DiceSynth::Process(double time)
{
   PROFILER(DiceSynth);

   mNoteInputBuffer.Process(time);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   float* outL = mWriteBuffer.GetChannel(0);
   float* outR = mWriteBuffer.GetChannel(1);

   //---- playback of the molded sample, with a short linear gain ramp to avoid clicks on play/stop ----
   float gainStart = mVoiceGain;
   float gainTarget = mPlaying ? mGain : 0.0f;
   int len = mHasMolded ? mMoldedLength : 0;
   const float* src = mHasMolded ? mMoldedSample.Data()->GetChannel(0) : nullptr;

   //playback stays inside the draggable region markers, and looping wraps at the end marker rather
   //than at the end of the buffer
   int regionStart = 0, regionEnd = len;
   if (len > 0)
   {
      float lo = MIN(mRegionStart, mRegionEnd);
      float hi = MAX(mRegionStart, mRegionEnd);
      regionStart = (int)ofClamp(lo * len, 0.0f, (float)(len - 1));
      regionEnd = (int)ofClamp(hi * len, 1.0f, (float)len);
      if (regionEnd <= regionStart)
         regionEnd = MIN(len, regionStart + 1);
   }

   for (int i = 0; i < bufferSize; ++i)
   {
      float g = gainStart + (gainTarget - gainStart) * ((float)(i + 1) / bufferSize);
      float sampleVal = 0.0f;

      if (mPlaying && len > 0)
      {
         if (mPlayPos < regionStart || mPlayPos >= regionEnd)
            mPlayPos = regionStart; //markers may have been dragged out from under the playhead

         int idx = (int)mPlayPos;
         int idx2 = (idx + 1 < regionEnd) ? idx + 1 : (mLoop ? regionStart : idx);
         float frac = (float)(mPlayPos - idx);
         sampleVal = src[idx] + (src[idx2] - src[idx]) * frac;

         mPlayPos += mPitchRatio;
         if (mPlayPos >= regionEnd)
         {
            if (mLoop)
               mPlayPos = regionStart + (mPlayPos - regionEnd);
            else
            {
               mPlayPos = regionEnd - 1;
               mPlaying = false;
            }
         }
      }

      outL[i] = sampleVal * g;
      outR[i] = outL[i];
   }
   mVoiceGain = gainTarget;

   //---- path RECORDING stays once per buffer: the mouse only moves at event rate, so sampling it
   //---- faster would just duplicate points and bloat the recording. ----
   if (mRecordingPath)
   {
      mRecordedPath.push_back({ mXYPos[0], mXYPos[1] });
      if (mRecordedPath.size() >= (size_t)kMaxPathPoints)
         mRecordingPath = false; //hit the safety cap - stop rather than grow forever
   }

   //---- CONTROL-RATE SUB-BLOCKS. Updating the XY position and the effect coefficients only once per
   //---- audio buffer (~5.8ms) means a path quantized to 8n gets barely 40 evaluations for the whole
   //---- loop, so the orb hops in straight lines between widely spaced points and visibly cuts the
   //---- corners of the recorded curve. Stepping the controls every kControlBlock samples traces the
   //---- path accurately at any playback speed, and costs almost nothing: the expensive sinf/powf
   //---- coefficient maths now runs a few thousand times a second instead of a few hundred.
   const int kControlBlock = 16;
   for (int blockStart = 0; blockStart < bufferSize; blockStart += kControlBlock)
   {
      const int blockEnd = MIN(bufferSize, blockStart + kControlBlock);
      const int blockLen = blockEnd - blockStart;

      //---- path playback, timed in SAMPLES so it is independent of buffer size and of how fast the
      //---- musical division asks the path to be traversed ----
      if (!mRecordingPath && mPlayingPath && !mRecordedPath.empty())
      {
         int rawSize = (int)mRecordedPath.size();
         double pathLenSamples = (double)rawSize * bufferSize; //recording captured one point per buffer
         if (mQuantizePath)
         {
            double targetMs = TheTransport->GetDuration(mQuantizeInterval);
            pathLenSamples = MAX(1.0, targetMs * 0.001 * gSampleRate);
         }

         double t = mPathPlaybackPhase / pathLenSamples;
         float rawIdxF = (float)ofClamp((float)t, 0.0f, 1.0f) * (rawSize - 1);
         int idx0 = (int)rawIdxF;
         int idx1 = (idx0 + 1 < rawSize) ? idx0 + 1 : idx0;
         float frac = rawIdxF - idx0;
         mXYPos[0] = mRecordedPath[idx0].x + (mRecordedPath[idx1].x - mRecordedPath[idx0].x) * frac;
         mXYPos[1] = mRecordedPath[idx0].y + (mRecordedPath[idx1].y - mRecordedPath[idx0].y) * frac;

         mPathPlaybackPhase += blockLen;
         if (mPathPlaybackPhase >= pathLenSamples)
         {
            if (mLoopPath)
               mPathPlaybackPhase -= pathLenSamples;
            else
            {
               mPathPlaybackPhase = pathLenSamples;
               mPlayingPath = false; //the checkbox is bound to this bool by pointer, so it reflects the change on next redraw
            }
         }
      }

      //---- live XY-blended effect chain: 10 effects sitting at fixed spots around the pad, blended by proximity ----
      float w[kNumLiveEffects];
      for (int e = 0; e < kNumLiveEffects; ++e)
         w[e] = ComputeEffectWeight(e);

      //each effect's "how extreme at full weight" comes from mEffectSpots' sibling parameters below,
      //which "randomize fx" reshuffles - the w=0 endpoint (mild/off) stays fixed for all of them
      float filterCutoff = mFilterMinCutoffHz + (1.0f - w[kFxLive_Filter]) * (7200.0f - mFilterMinCutoffHz);
      float filterF = 2.0f * sinf((float)PI * filterCutoff / gSampleRate);
      //comb: pad position picks the resonant pitch, weight sets how hard it rings
      int combDelay = (int)(gSampleRate / ofLerp(900.0f, 90.0f, mXYPos[1]));
      combDelay = ofClamp(combDelay, 2, kCombBufSize - 1);
      float combFeedback = w[kFxLive_Comb] * mCombMaxFeedback;
      //chorus: weight sets sweep depth
      float chorusDepth = w[kFxLive_Chorus] * mChorusMaxDepthMs * 0.001f * gSampleRate;
      float chorusBase = gSampleRate * 0.012f;
      float pitchShiftRate = 1.0f + w[kFxLive_Pitch] * mPitchMaxRateBoost;
      float freqShiftPhaseInc = (float)TWO_PI * (w[kFxLive_FreqShift] * mFreqShiftMaxHz) / gSampleRate;
      int delayTimeSamples = MAX(10, (int)(w[kFxLive_Delay] * mDelayMaxTimeMs * 0.001f * gSampleRate));
      if (delayTimeSamples >= kDelayBufSize)
         delayTimeSamples = kDelayBufSize - 1;
      float delayFeedback = w[kFxLive_Delay] * mDelayMaxFeedback;
      float reverbFeedback = w[kFxLive_Reverb] * mReverbMaxFeedback;
      float stutterMs = 90.0f + w[kFxLive_Stutter] * (mStutterMinLenMs - 90.0f);
      int stutterLen = MAX(64, (int)(gSampleRate * 0.001f * stutterMs));
      if (stutterLen >= kStutterBufSize)
         stutterLen = kStutterBufSize - 1;
      float repeaterMs = 400.0f + w[kFxLive_Repeater] * (mRepeaterMinLenMs - 400.0f);
      int repeaterLen = MAX(256, (int)(gSampleRate * 0.001f * repeaterMs));
      if (repeaterLen >= kRepeaterBufSize)
         repeaterLen = kRepeaterBufSize - 1;
      int reverseWindow = MAX(64, (int)(gSampleRate * 0.001f * w[kFxLive_Reverse] * mReverseMaxWindowMs));
      if (reverseWindow >= kReverseBufSize)
         reverseWindow = kReverseBufSize - 1;

      for (int i = blockStart; i < blockEnd; ++i)
      {
         float dryL = outL[i];
         float dryR = outR[i];
         float wetL = dryL;
         float wetR = dryR;

         //1. filter
         if (w[kFxLive_Filter] > 0.02f)
         {
            mFilterLow[0] += filterF * mFilterBand[0];
            float highL = wetL - mFilterLow[0] - 0.5f * mFilterBand[0];
            mFilterBand[0] += filterF * highL;

            mFilterLow[1] += filterF * mFilterBand[1];
            float highR = wetR - mFilterLow[1] - 0.5f * mFilterBand[1];
            mFilterBand[1] += filterF * highR;

            wetL = wetL * (1.0f - w[kFxLive_Filter]) + mFilterLow[0] * w[kFxLive_Filter];
            wetR = wetR * (1.0f - w[kFxLive_Filter]) + mFilterLow[1] * w[kFxLive_Filter];
         }
         else
         {
            mFilterLow[0] *= 0.9f;
            mFilterBand[0] *= 0.9f;
            mFilterLow[1] *= 0.9f;
            mFilterBand[1] *= 0.9f;
         }

         //2. comb resonator - tuned ringing
         {
            int readPos = (mCombWritePos - combDelay + kCombBufSize) % kCombBufSize;
            float cl = wetL + mCombBuf[0][readPos] * combFeedback;
            float cr = wetR + mCombBuf[1][readPos] * combFeedback;
            mCombBuf[0][mCombWritePos] = cl;
            mCombBuf[1][mCombWritePos] = cr;
            mCombWritePos = (mCombWritePos + 1) % kCombBufSize;
            if (w[kFxLive_Comb] > 0.02f)
            {
               float trim = 1.0f - combFeedback * 0.5f; //keep the level in check as it resonates
               wetL = wetL * (1.0f - w[kFxLive_Comb]) + cl * trim * w[kFxLive_Comb];
               wetR = wetR * (1.0f - w[kFxLive_Comb]) + cr * trim * w[kFxLive_Comb];
            }
         }

         //3. chorus - swept short delay for movement and width
         {
            mChorusBuf[0][mChorusWritePos] = wetL;
            mChorusBuf[1][mChorusWritePos] = wetR;
            if (w[kFxLive_Chorus] > 0.02f)
            {
               //the two channels read opposite sides of the sweep, which widens the image
               for (int ch = 0; ch < 2; ++ch)
               {
                  float lfo = sinf((mChorusPhase + (ch ? 0.25f : 0.0f)) * (float)TWO_PI);
                  float d = ofClamp(chorusBase + chorusDepth * lfo, 1.0f, (float)(kChorusBufSize - 2));
                  float readPos = (float)mChorusWritePos - d;
                  while (readPos < 0)
                     readPos += kChorusBufSize;
                  int r0 = (int)readPos % kChorusBufSize;
                  int r1 = (r0 + 1) % kChorusBufSize;
                  float f = readPos - floorf(readPos);
                  float voice = mChorusBuf[ch][r0] * (1.0f - f) + mChorusBuf[ch][r1] * f;
                  float& target = (ch == 0) ? wetL : wetR;
                  target = target * (1.0f - w[kFxLive_Chorus] * 0.5f) + voice * w[kFxLive_Chorus] * 0.5f;
               }
            }
            mChorusWritePos = (mChorusWritePos + 1) % kChorusBufSize;
            mChorusPhase += 0.6f / gSampleRate;
            if (mChorusPhase >= 1.0f)
               mChorusPhase -= 1.0f;
         }

         //4. pitch shift (granular delay)
         mPitchBuf[0][mPitchWritePos] = wetL;
         mPitchBuf[1][mPitchWritePos] = wetR;
         if (w[kFxLive_Pitch] > 0.02f)
         {
            mPitchReadPos[0] += pitchShiftRate;
            mPitchReadPos[1] += pitchShiftRate;
            if (mPitchReadPos[0] >= kPitchBufSize)
               mPitchReadPos[0] -= (kPitchBufSize / 2);
            if (mPitchReadPos[1] >= kPitchBufSize)
               mPitchReadPos[1] -= (kPitchBufSize / 2);

            int rIdx0 = ((int)mPitchReadPos[0]) % kPitchBufSize;
            int rIdx1 = ((int)mPitchReadPos[1]) % kPitchBufSize;
            wetL = wetL * (1.0f - w[kFxLive_Pitch]) + mPitchBuf[0][rIdx0] * w[kFxLive_Pitch];
            wetR = wetR * (1.0f - w[kFxLive_Pitch]) + mPitchBuf[1][rIdx1] * w[kFxLive_Pitch];
         }
         mPitchWritePos = (mPitchWritePos + 1) % kPitchBufSize;

         //5. freq shift (ring-mod approximation)
         if (w[kFxLive_FreqShift] > 0.02f)
         {
            float mod = cosf(mFreqShiftPhase);
            wetL = wetL * (1.0f - w[kFxLive_FreqShift]) + (wetL * mod) * w[kFxLive_FreqShift];
            wetR = wetR * (1.0f - w[kFxLive_FreqShift]) + (wetR * mod) * w[kFxLive_FreqShift];
         }
         mFreqShiftPhase += freqShiftPhaseInc;
         if (mFreqShiftPhase > (float)TWO_PI)
            mFreqShiftPhase -= (float)TWO_PI;

         //6. delay
         {
            int readPos = (mDelayWritePos - delayTimeSamples + kDelayBufSize) % kDelayBufSize;
            float dl = mDelayBuf[0][readPos];
            float dr = mDelayBuf[1][readPos];
            if (w[kFxLive_Delay] > 0.02f)
            {
               mDelayBuf[0][mDelayWritePos] = wetL + dl * delayFeedback;
               mDelayBuf[1][mDelayWritePos] = wetR + dr * delayFeedback;
               wetL = wetL * (1.0f - w[kFxLive_Delay]) + dl * w[kFxLive_Delay];
               wetR = wetR * (1.0f - w[kFxLive_Delay]) + dr * w[kFxLive_Delay];
            }
            else
            {
               mDelayBuf[0][mDelayWritePos] = wetL;
               mDelayBuf[1][mDelayWritePos] = wetR;
            }
            mDelayWritePos = (mDelayWritePos + 1) % kDelayBufSize;
         }

         //7. reverb (3-comb)
         {
            mRevBuf1[0][mRevIdx1] = wetL + mRevBuf1[0][mRevIdx1] * reverbFeedback;
            mRevBuf2[0][mRevIdx2] = wetL + mRevBuf2[0][mRevIdx2] * reverbFeedback;
            mRevBuf3[0][mRevIdx3] = wetL + mRevBuf3[0][mRevIdx3] * reverbFeedback;
            mRevBuf1[1][mRevIdx1] = wetR + mRevBuf1[1][mRevIdx1] * reverbFeedback;
            mRevBuf2[1][mRevIdx2] = wetR + mRevBuf2[1][mRevIdx2] * reverbFeedback;
            mRevBuf3[1][mRevIdx3] = wetR + mRevBuf3[1][mRevIdx3] * reverbFeedback;

            if (w[kFxLive_Reverb] > 0.02f)
            {
               float revL = (mRevBuf1[0][mRevIdx1] + mRevBuf2[0][mRevIdx2] + mRevBuf3[0][mRevIdx3]) * 0.33f;
               float revR = (mRevBuf1[1][mRevIdx1] + mRevBuf2[1][mRevIdx2] + mRevBuf3[1][mRevIdx3]) * 0.33f;
               wetL = wetL * (1.0f - w[kFxLive_Reverb]) + revL * w[kFxLive_Reverb];
               wetR = wetR * (1.0f - w[kFxLive_Reverb]) + revR * w[kFxLive_Reverb];
            }
            mRevIdx1 = (mRevIdx1 + 1) % kRevDel1;
            mRevIdx2 = (mRevIdx2 + 1) % kRevDel2;
            mRevIdx3 = (mRevIdx3 + 1) % kRevDel3;
         }

         //8. stutter (short, fast repeats)
         mStutterBuf[0][mStutterWritePos] = wetL;
         mStutterBuf[1][mStutterWritePos] = wetR;
         if (w[kFxLive_Stutter] > 0.02f)
         {
            mStutterReadPos = (mStutterReadPos + 1) % stutterLen;
            int readIdx = (mStutterWritePos - stutterLen + mStutterReadPos + kStutterBufSize) % kStutterBufSize;
            wetL = wetL * (1.0f - w[kFxLive_Stutter]) + mStutterBuf[0][readIdx] * w[kFxLive_Stutter];
            wetR = wetR * (1.0f - w[kFxLive_Stutter]) + mStutterBuf[1][readIdx] * w[kFxLive_Stutter];
         }
         mStutterWritePos = (mStutterWritePos + 1) % kStutterBufSize;

         //9. repeater (longer, coarser loop - "broken record")
         mRepeaterBuf[0][mRepeaterWritePos] = wetL;
         mRepeaterBuf[1][mRepeaterWritePos] = wetR;
         if (w[kFxLive_Repeater] > 0.02f)
         {
            mRepeaterReadPos = (mRepeaterReadPos + 1) % repeaterLen;
            int readIdx = (mRepeaterWritePos - repeaterLen + mRepeaterReadPos + kRepeaterBufSize) % kRepeaterBufSize;
            wetL = wetL * (1.0f - w[kFxLive_Repeater]) + mRepeaterBuf[0][readIdx] * w[kFxLive_Repeater];
            wetR = wetR * (1.0f - w[kFxLive_Repeater]) + mRepeaterBuf[1][readIdx] * w[kFxLive_Repeater];
         }
         mRepeaterWritePos = (mRepeaterWritePos + 1) % kRepeaterBufSize;

         //10. reverse (recent history scanned backwards, on repeat)
         mReverseBuf[0][mReverseWritePos] = wetL;
         mReverseBuf[1][mReverseWritePos] = wetR;
         if (w[kFxLive_Reverse] > 0.02f)
         {
            mReverseReadOffset++;
            if (mReverseReadOffset >= reverseWindow)
               mReverseReadOffset = 0;
            int readIdx = ((mReverseWritePos - mReverseReadOffset) % kReverseBufSize + kReverseBufSize) % kReverseBufSize;
            wetL = wetL * (1.0f - w[kFxLive_Reverse]) + mReverseBuf[0][readIdx] * w[kFxLive_Reverse];
            wetR = wetR * (1.0f - w[kFxLive_Reverse]) + mReverseBuf[1][readIdx] * w[kFxLive_Reverse];
         }
         mReverseWritePos = (mReverseWritePos + 1) % kReverseBufSize;

         //master wet/dry for the whole 10-effect chain, independent of where the XY pad sits
         outL[i] = dryL * (1.0f - mFxMix) + wetL * mFxMix;
         outR[i] = dryR * (1.0f - mFxMix) + wetR * mFxMix;
      }
   }

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), bufferSize);
   }
}

void DiceSynth::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time))
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity > 0)
   {
      mPitchRatio = powf(2.0f, (note.pitch - mRootPitch) / 12.0f);
      mPlayPos = 0;
      mPlaying = true;
   }
   else if (!mLoop)
   {
      //one-shot: let it finish naturally, note-off doesn't cut it
   }
   else
   {
      mPlaying = false;
   }
}

void DiceSynth::SampleDropped(int x, int y, Sample* sample)
{
   if (sample == nullptr)
      return;

   mSourceSample.CopyFrom(sample);
   mOriginalSample.CopyFrom(sample); //pristine, so iteration is always reversible
   mGeneration = 0;
   mHasDroppedSample = true;
   mRegionStart = 0.0f; //a new sample starts fully selected rather than inheriting the last one's region
   mRegionEnd = 1.0f;
   mNeedsReanalysis = true; //a new sample needs fresh pitch/harmonic analysis before the next roll
   mGenomeValid = false;
   EnsureMoldedCapacity(); //allocate now, while nothing is playing this buffer yet

   //mold it right away - otherwise dropping a new sample in has no visible/audible effect until
   //the user thinks to click the die, which reads as "the drop didn't work"
   RollDice();
}

void DiceSynth::SeekWaveformTo(float x)
{
   if (!mHasMolded)
      return;

   int len = mMoldedLength;
   if (len <= 0)
      return;

   float frac = ofClamp((x - kWaveX) / kWaveW, 0.0f, 1.0f);
   //clamp into the region, otherwise playback would immediately snap away from where you clicked
   frac = ofClamp(frac, MIN(mRegionStart, mRegionEnd), MAX(mRegionStart, mRegionEnd));
   mPlayPos = (double)frac * len;
   mPitchRatio = 1.0f;
   mPlaying = true;
}

void DiceSynth::OnClicked(float x, float y, bool right)
{
   //the die itself - hand-drawn, so it gets a manual hit test rather than a generic button
   if (!right && x >= kDiceX && x <= kDiceX + kDiceSize && y >= kDiceY && y <= kDiceY + kDiceSize)
   {
      RollDice();
      return;
   }

   if (!right && x >= kWaveX && x <= kWaveX + kWaveW && y >= kWaveY && y <= kWaveY + kWaveH)
   {
      //grabbing a region marker takes priority over scrubbing, so the markers stay reachable even
      //where they sit on top of the waveform
      const float grabPx = 7.0f;
      float startX = kWaveX + kWaveW * ofClamp(mRegionStart, 0.0f, 1.0f);
      float endX = kWaveX + kWaveW * ofClamp(mRegionEnd, 0.0f, 1.0f);
      if (fabsf(x - startX) <= grabPx && fabsf(x - startX) <= fabsf(x - endX))
      {
         mDragMarker = 1;
         return;
      }
      if (fabsf(x - endX) <= grabPx)
      {
         mDragMarker = 2;
         return;
      }

      mScrubbingWaveform = true;
      SeekWaveformTo(x);
      return;
   }

   if (!right && x >= kXYPadX && x <= kXYPadX + kXYPadSize && y >= kXYPadY && y <= kXYPadY + kXYPadSize)
   {
      mDraggingXY = true;
      mXYPos[0] = ofClamp((x - kXYPadX) / kXYPadSize, 0.0f, 1.0f);
      mXYPos[1] = ofClamp((y - kXYPadY) / kXYPadSize, 0.0f, 1.0f);
      return;
   }

   IDrawableModule::OnClicked(x, y, right);
}

bool DiceSynth::MouseMoved(float x, float y)
{
   if (mDraggingXY)
   {
      mXYPos[0] = ofClamp((x - kXYPadX) / kXYPadSize, 0.0f, 1.0f);
      mXYPos[1] = ofClamp((y - kXYPadY) / kXYPadSize, 0.0f, 1.0f);
   }
   if (mDragMarker != 0)
   {
      float frac = ofClamp((x - kWaveX) / kWaveW, 0.0f, 1.0f);
      const float minGap = 0.005f; //keep a sliver of region so playback always has somewhere to go
      if (mDragMarker == 1)
         mRegionStart = MIN(frac, mRegionEnd - minGap);
      else
         mRegionEnd = MAX(frac, mRegionStart + minGap);
   }
   else if (mScrubbingWaveform)
      SeekWaveformTo(x);
   return IDrawableModule::MouseMoved(x, y);
}

void DiceSynth::MouseReleased()
{
   mDraggingXY = false;
   mScrubbingWaveform = false;
   mDragMarker = 0;
   IDrawableModule::MouseReleased();
}

void DiceSynth::Poll()
{
}

void DiceSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void DiceSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mMoldModeSelector && mMoldMode != oldVal && mHasDroppedSample)
      RollDice(); //switching engine should immediately give you a sound from that engine
}

void DiceSynth::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPlayButton)
   {
      if (!mHasMolded)
         RollDice(); //nothing to play yet - roll first so Play always does something
      mPlayPos = 0;
      mPitchRatio = 1.0f;
      mPlaying = true;
   }
   else if (button == mStopButton)
   {
      mPlaying = false;
   }
   else if (button == mRandomizeEffectsButton)
   {
      RandomizeEffects();
   }
   else if (button == mIterateButton)
   {
      IterateMold();
   }
   else if (button == mResetButton)
   {
      ResetToOriginal();
   }
}

void DiceSynth::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRecordPathCheckbox)
   {
      if (mRecordingPath)
      {
         mRecordedPath.clear();
         if (mPlayingPath)
         {
            mPlayingPath = false;
            mPlayPathCheckbox->SetValue(0.0f, time);
         }
      }
   }
   else if (checkbox == mPlayPathCheckbox)
   {
      if (mPlayingPath)
      {
         mPathPlaybackPhase = 0.0;
         if (mRecordingPath)
         {
            mRecordingPath = false;
            mRecordPathCheckbox->SetValue(0.0f, time);
         }
      }
   }
}

void DiceSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void DiceSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void DiceSynth::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mHasDroppedSample;
   if (mHasDroppedSample)
      mSourceSample.SaveState(out);

   out << mHasMolded;
   if (mHasMolded)
      mMoldedSample.SaveState(out);
   //the molded buffer is deliberately over-allocated, so how much of it is real audio has to be
   //recorded separately - without this a reloaded patch plays the whole capacity, silence included
   out << mMoldedLength;

   out << mLastRoll;

   out << (int)mRecordedPath.size();
   for (const auto& p : mRecordedPath)
   {
      out << p.x;
      out << p.y;
   }

   for (int e = 0; e < kNumLiveEffects; ++e)
   {
      out << mEffectSpots[e].x;
      out << mEffectSpots[e].y;
   }
   out << mFilterMinCutoffHz;
   out << mCombMaxFeedback;
   out << mChorusMaxDepthMs;
   out << mPitchMaxRateBoost;
   out << mFreqShiftMaxHz;
   out << mDelayMaxTimeMs;
   out << mDelayMaxFeedback;
   out << mReverbMaxFeedback;
   out << mStutterMinLenMs;
   out << mRepeaterMinLenMs;
   out << mReverseMaxWindowMs;

   //the genome represents however many generations of mutation you've rolled through - without
   //this, reloading a patch would silently reset that lineage back to the un-mutated baseline
   out << mGenomeValid;
   //explicit counts: the harmonic/band counts are tuning constants that may change again, and
   //without them a future change silently misreads every older save file
   out << (int)kNumHarmonics;
   for (int h = 0; h < kNumHarmonics; ++h)
      out << mGenome.harmonicAmp[h];
   out << (int)kNumNoiseBands;
   for (int b = 0; b < kNumNoiseBands; ++b)
      out << mGenome.noiseBandAmp[b];
   out << mGenome.noiseAmount;
   out << mGenome.baselineNoiseAmount;
   out << mGenome.attackScale;
   out << mGenome.decayScale;
   out << mGenome.brightnessTilt;
   out << mGenome.inharmonicity;
   out << mGenome.pitchShiftSemitones;
   out << mGenome.harmonicStretch;
   out << mGenome.reverseResidual;

   out << mRegionStart;
   out << mRegionEnd;
   out << mChaos;
}

void DiceSynth::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 1)
   {
      in >> mHasDroppedSample;
      if (mHasDroppedSample)
      {
         mSourceSample.LoadState(in);
         //a third full copy in every patch is not worth the file size, so reset returns to whatever
         //source was saved rather than to a pre-iteration original
         mOriginalSample.CopyFrom(&mSourceSample);
      }

      in >> mHasMolded;
      if (mHasMolded)
         mMoldedSample.LoadState(in);
      if (rev >= 8)
         in >> mMoldedLength;
      else
         mMoldedLength = mMoldedSample.LengthInSamples(); //older files stored exactly what they used
      mMoldedLength = MIN(mMoldedLength, mMoldedSample.LengthInSamples());

      in >> mLastRoll;
   }

   if (rev >= 2)
   {
      int size;
      in >> size;
      //sanity-clamp before allocating - a corrupted or hand-edited save file could otherwise
      //drive a huge or negative count here
      LoadStateValidate(size >= 0 && size <= kMaxPathPoints);
      mRecordedPath.resize(size);
      for (int i = 0; i < size; ++i)
      {
         in >> mRecordedPath[i].x;
         in >> mRecordedPath[i].y;
      }
   }

   if (rev >= 3)
   {
      for (int e = 0; e < kNumLiveEffects; ++e)
      {
         in >> mEffectSpots[e].x;
         in >> mEffectSpots[e].y;
      }
      in >> mFilterMinCutoffHz;
      in >> mCombMaxFeedback;
      in >> mChorusMaxDepthMs;
      in >> mPitchMaxRateBoost;
      in >> mFreqShiftMaxHz;
      in >> mDelayMaxTimeMs;
      in >> mDelayMaxFeedback;
      in >> mReverbMaxFeedback;
      in >> mStutterMinLenMs;
      in >> mRepeaterMinLenMs;
      in >> mReverseMaxWindowMs;
   }

   if (rev >= 6)
   {
      in >> mGenomeValid;

      int savedHarmonics;
      in >> savedHarmonics;
      LoadStateValidate(savedHarmonics >= 0 && savedHarmonics <= 4096);
      for (int h = 0; h < savedHarmonics; ++h)
      {
         float v;
         in >> v;
         if (h < kNumHarmonics)
            mGenome.harmonicAmp[h] = v; //extra partials in an older/newer file are simply dropped
      }

      int savedBands;
      in >> savedBands;
      LoadStateValidate(savedBands >= 0 && savedBands <= 4096);
      for (int b = 0; b < savedBands; ++b)
      {
         float v;
         in >> v;
         if (b < kNumNoiseBands)
            mGenome.noiseBandAmp[b] = v;
      }

      in >> mGenome.noiseAmount;
      in >> mGenome.baselineNoiseAmount;
      in >> mGenome.attackScale;
      in >> mGenome.decayScale;
      in >> mGenome.brightnessTilt;
      in >> mGenome.inharmonicity;
      in >> mGenome.pitchShiftSemitones;
   }

   if (rev >= 7)
   {
      in >> mGenome.harmonicStretch;
      in >> mGenome.reverseResidual;
   }

   if (rev >= 9)
   {
      in >> mRegionStart;
      in >> mRegionEnd;
   }

   if (rev >= 10)
      in >> mChaos;
   else if (rev >= 4)
   {
      //revs 4-5 wrote a fixed-length genome block, but the harmonic count changed mid-rev-5, so the
      //block can't be parsed unambiguously. Consume it without interpreting it (the container checks
      //for a separator right after this, so the bytes must still be read) and re-derive the genome
      //from analysis instead - that loses an evolved lineage from an old save, which beats
      //desyncing the rest of the patch file.
      while (ModuleContainer::DoesModuleHaveMoreSaveData(in))
      {
         char discard;
         in >> discard;
      }
      mGenomeValid = false;
   }

   //mAnalysis is never persisted (large, and fully derivable from mSourceSample) - always rebuild it
   //before the next roll; mGenomeValid (just loaded above, if this file has it) decides whether that
   //rebuild is allowed to reset the genome back to baseline or must leave the loaded lineage alone
   mNeedsReanalysis = true;
}

void DiceSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMoldModeSelector->Draw();
   mChaosSlider->Draw();

   //sample section
   mPlayButton->Draw();
   mStopButton->Draw();
   mLoopCheckbox->Draw();
   mGainSlider->Draw();

   mIterateButton->Draw();
   mResetButton->Draw();
   if (mGeneration > 0)
   {
      ofPushStyle();
      ofSetColor(200, 200, 210, 200);
      DrawTextNormal("gen " + ofToString(mGeneration), kGenTextX, kIterY + 13, 11);
      ofPopStyle();
   }

   //fx section
   mRandomizeEffectsButton->Draw();
   mFxMixSlider->Draw();
   mRecordPathCheckbox->Draw();
   mPlayPathCheckbox->Draw();
   mLoopPathCheckbox->Draw();
   mQuantizePathCheckbox->Draw();
   mQuantizeIntervalSelector->Draw();

   //divider so it reads as two groups: what shapes the sample above, what drives the fx pad below
   ofPushStyle();
   ofSetColor(90, 90, 100, 140);
   ofSetLineWidth(1);
   ofLine(kDiceX, kDividerY, mWidth - 6, kDividerY);
   ofPopStyle();

   //---- the die ----
   ofPushStyle();
   ofFill();
   ofSetColor(235, 235, 240);
   ofRect(kDiceX, kDiceY, kDiceSize, kDiceSize, 10);
   ofNoFill();
   ofSetColor(40, 40, 45);
   ofSetLineWidth(2);
   ofRect(kDiceX, kDiceY, kDiceSize, kDiceSize, 10);

   ofFill();
   ofSetColor(30, 30, 35);
   const float cx = kDiceX + kDiceSize * 0.5f;
   const float cy = kDiceY + kDiceSize * 0.5f;
   const float o = kDiceSize * 0.26f; //pip offset from center
   const float r = kDiceSize * 0.07f; //pip radius
   int roll = mLastRoll > 0 ? mLastRoll : 1;
   bool showCenter = (roll % 2) == 1; //1, 3, 5
   bool showCorners = roll >= 2; //2, 3, 4, 5, 6
   bool showMids = roll >= 4; //4, 5, 6
   bool showSides = roll == 6;
   if (showCenter)
      ofCircle(cx, cy, r);
   if (showCorners)
   {
      ofCircle(cx - o, cy - o, r);
      ofCircle(cx + o, cy + o, r);
   }
   if (showMids)
   {
      ofCircle(cx - o, cy + o, r);
      ofCircle(cx + o, cy - o, r);
   }
   if (showSides)
   {
      ofCircle(cx - o, cy, r);
      ofCircle(cx + o, cy, r);
   }
   ofPopStyle();

   //---- waveform of the currently molded / playing sample ----
   ofPushStyle();
   ofSetColor(12, 14, 18, 220);
   ofFill();
   ofRect(kWaveX, kWaveY, kWaveW, kWaveH);
   if (mHasMolded)
   {
      int len = mMoldedLength;
      if (len > 0)
      {
         const float* data = mMoldedSample.Data()->GetChannel(0);
         //colour keys the waveform to the active mold mode, so which engine produced this is
         //readable at a glance rather than requiring a look at the dropdown
         if (mMoldMode == kMold_Resynth)
            ofSetColor(255, 185, 90);
         else
            ofSetColor(90, 200, 255);
         ofSetLineWidth(1);
         const float midY = kWaveY + kWaveH * 0.5f;
         int steps = (int)kWaveW;
         for (int i = 0; i < steps; ++i)
         {
            int a = (int)((float)i / steps * len);
            int b = (int)((float)(i + 1) / steps * len);
            if (b <= a)
               b = a + 1;
            if (b > len)
               b = len;
            int stride = MAX(1, (b - a) / 64);
            float mn = 1.0f, mx = -1.0f;
            for (int j = a; j < b; j += stride)
            {
               float s = data[j];
               if (s < mn)
                  mn = s;
               if (s > mx)
                  mx = s;
            }
            float y0 = midY - ofClamp(mx, -1.0f, 1.0f) * kWaveH * 0.46f;
            float y1 = midY - ofClamp(mn, -1.0f, 1.0f) * kWaveH * 0.46f;
            ofLine(kWaveX + i, y0, kWaveX + i, y1 + 0.5f);
         }

         if (mPlaying)
         {
            float px = kWaveX + kWaveW * ofClamp((float)(mPlayPos / len), 0.0f, 1.0f);
            ofSetColor(255, 210, 90, 220);
            ofLine(px, kWaveY, px, kWaveY + kWaveH);
         }

         //---- play region: darken what won't be heard, then draw the two draggable markers ----
         {
            float sx = kWaveX + kWaveW * ofClamp(mRegionStart, 0.0f, 1.0f);
            float ex = kWaveX + kWaveW * ofClamp(mRegionEnd, 0.0f, 1.0f);
            ofPushStyle();
            ofFill();
            ofSetColor(0, 0, 0, 140);
            if (sx > kWaveX)
               ofRect(kWaveX, kWaveY, sx - kWaveX, kWaveH);
            if (ex < kWaveX + kWaveW)
               ofRect(ex, kWaveY, (kWaveX + kWaveW) - ex, kWaveH);

            ofSetLineWidth(1);
            ofSetColor(90, 255, 150, 230); //start
            ofLine(sx, kWaveY, sx, kWaveY + kWaveH);
            ofRect(sx, kWaveY, 3, 5);
            ofSetColor(255, 120, 120, 230); //end
            ofLine(ex, kWaveY, ex, kWaveY + kWaveH);
            ofRect(ex - 3, kWaveY, 3, 5);
            ofPopStyle();
         }
      }
   }
   else
   {
      ofSetColor(150, 150, 150);
      DrawTextNormal("drag a sample in to get started", kWaveX + 8, kWaveY + kWaveH * 0.5f, 12);
   }
   ofPopStyle();

   //---- XY pad ----
   ofPushStyle();
   ofSetColor(18, 20, 26, 220);
   ofFill();
   ofRect(kXYPadX, kXYPadY, kXYPadSize, kXYPadSize);
   ofNoFill();
   ofSetColor(60, 60, 68);
   ofRect(kXYPadX, kXYPadY, kXYPadSize, kXYPadSize);

   if (!mRecordedPath.empty())
   {
      ofNoFill();
      ofSetColor(120, 120, 130, 160);
      ofBeginShape();
      for (const auto& p : mRecordedPath)
         ofVertex(kXYPadX + kXYPadSize * p.x, kXYPadY + kXYPadSize * p.y);
      ofEndShape();
   }

   float dotX = kXYPadX + kXYPadSize * mXYPos[0];
   float dotY = kXYPadY + kXYPadSize * mXYPos[1];
   ofFill();
   if (mRecordingPath)
      ofSetColor(255, 90, 90, 230);
   else if (mPlayingPath)
      ofSetColor(120, 255, 150, 230);
   else
      ofSetColor(255, 255, 255, 230);
   ofCircle(dotX, dotY, 6);
   ofNoFill();
   ofSetColor(255, 220, 120, 200);
   ofSetLineWidth(2);
   ofCircle(dotX, dotY, 9);
   ofPopStyle();
}

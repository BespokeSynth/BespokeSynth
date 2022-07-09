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
//  StepSequencer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#include "StepSequencer.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "MidiController.h"

namespace
{
   const int kMetaStepLoop = 8;
}

StepSequencer::StepSequencer()
: mFlusher(this)
{
   mFlusher.SetInterval(mStepInterval);

   mMetaStepMasks = new juce::uint32[META_STEP_MAX * NUM_STEPSEQ_ROWS];
   for (int i = 0; i < META_STEP_MAX * NUM_STEPSEQ_ROWS; ++i)
      mMetaStepMasks[i] = 0xff;
}

void StepSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mStepInterval, OffsetInfo(0, true), true);
}

void StepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid("uigrid", 40, 45, 250, 150, 16, NUM_STEPSEQ_ROWS, this);
   mStrengthSlider = new FloatSlider(this, "vel", 87, 22, 70, 15, &mStrength, 0, 1, 2);
   mRandomizeButton = new ClickButton(this, "randomize", 160, 22);
   mRandomizationDensitySlider = new FloatSlider(this, "r den", mRandomizeButton, kAnchor_Right, 65, 15, &mRandomizationDensity, 0, 1, 2);
   mRandomizationAmountSlider = new FloatSlider(this, "r amt", mRandomizationDensitySlider, kAnchor_Right, 65, 15, &mRandomizationAmount, 0, 1, 2);
   mNumMeasuresSlider = new IntSlider(this, "measures", 5, 22, 80, 15, &mNumMeasures, 1, 4);
   mPresetDropdown = new DropdownList(this, "preset", 5, 4, &mPreset);
   mGridYOffDropdown = new DropdownList(this, "yoff", 295, 4, &mGridYOff);
   mAdjustOffsetsCheckbox = new Checkbox(this, "offsets", 175, 4, &mAdjustOffsets);
   mRepeatRateDropdown = new DropdownList(this, "repeat", 155, 22, (int*)(&mRepeatRate));
   mStepIntervalDropdown = new DropdownList(this, "step", 133, 4, (int*)(&mStepInterval));
   mCurrentColumnSlider = new IntSlider(this, "column", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 100, 15, &mCurrentColumn, 0, 15);
   mShiftLeftButton = new ClickButton(this, "<", 80, 4);
   mShiftRightButton = new ClickButton(this, ">", 100, 4);
   mGridControlTarget = new GridControlTarget(this, "grid", 240, 4);
   mVelocityGridController = new GridControlTarget(this, "velocity", 240, 16);
   mMetaStepGridController = new GridControlTarget(this, "metastep", 240, 28);

   mGrid->SetMajorColSize(4);
   mGrid->SetFlip(true);

   mPresetDropdown->AddLabel("clear", -1);
   mPresetDropdown->AddLabel("16s", 0);
   mPresetDropdown->AddLabel("8s", 1);
   mPresetDropdown->AddLabel("kicksnare", 2);
   mPresetDropdown->AddLabel("amen", 3);
   mPresetDropdown->AddLabel("boogaloo", 4);
   mPresetDropdown->AddLabel("dubstep", 5);
   mPresetDropdown->AddLabel("trades", 6);

   mGridYOffDropdown->AddLabel("0", 0);
   mGridYOffDropdown->AddLabel("1", 1);
   mGridYOffDropdown->AddLabel("2", 2);
   mGridYOffDropdown->AddLabel("3", 3);

   mVelocityGridController->SetShowing(false);
   mMetaStepGridController->SetShowing(false);

   for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
   {
      mRows[i] = new StepSequencerRow(this, mGrid, i);
      mRows[i]->CreateUIControls();
      mOffsets[i] = 0;
      mOffsetSlider[i] = new FloatSlider(this, ("offset" + ofToString(i)).c_str(), 230, 185 - i * 9.4f, 90, 9, &mOffsets[i], -1, 1);
      mRandomizeRowButton[i] = new ClickButton(this, ("random" + ofToString(i)).c_str(), mGridYOffDropdown->GetRect().getMaxX() + 4 + i * 60, 3);
      mNoteRepeats[i] = new NoteRepeat(this, i);
   }

   mRepeatRateDropdown->AddLabel("no repeat", kInterval_None);
   mRepeatRateDropdown->AddLabel("4n", kInterval_4n);
   mRepeatRateDropdown->AddLabel("4nt", kInterval_4nt);
   mRepeatRateDropdown->AddLabel("8n", kInterval_8n);
   mRepeatRateDropdown->AddLabel("8nt", kInterval_8nt);
   mRepeatRateDropdown->AddLabel("16n", kInterval_16n);
   mRepeatRateDropdown->AddLabel("16nt", kInterval_16nt);
   mRepeatRateDropdown->AddLabel("32n", kInterval_32n);
   mRepeatRateDropdown->AddLabel("32nt", kInterval_32nt);
   mRepeatRateDropdown->AddLabel("64n", kInterval_64n);

   mStepIntervalDropdown->AddLabel("4n", kInterval_4n);
   mStepIntervalDropdown->AddLabel("4nt", kInterval_4nt);
   mStepIntervalDropdown->AddLabel("8n", kInterval_8n);
   mStepIntervalDropdown->AddLabel("8nt", kInterval_8nt);
   mStepIntervalDropdown->AddLabel("16n", kInterval_16n);
   mStepIntervalDropdown->AddLabel("16nt", kInterval_16nt);
   mStepIntervalDropdown->AddLabel("32n", kInterval_32n);
   mStepIntervalDropdown->AddLabel("32nt", kInterval_32nt);
   mStepIntervalDropdown->AddLabel("64n", kInterval_64n);
}

StepSequencer::~StepSequencer()
{
   TheTransport->RemoveListener(this);

   for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
   {
      delete mRows[i];
      delete mNoteRepeats[i];
   }

   delete[] mMetaStepMasks;
}

void StepSequencer::Poll()
{
   IDrawableModule::Poll();

   ComputeSliders(0);

   if (HasGridController())
   {
      int numChunks = GetNumControllerChunks();
      if (numChunks != mGridYOffDropdown->GetNumValues())
      {
         mGridYOffDropdown->Clear();
         for (int i = 0; i < numChunks; ++i)
            mGridYOffDropdown->AddLabel(ofToString(i).c_str(), i);
      }

      UpdateLights();
   }
}

namespace
{
   const float kMidwayVelocity = .75f;
}

void StepSequencer::UpdateLights(bool force /*=false*/)
{
   if (!HasGridController() || mGridControlTarget->GetGridController() == nullptr)
      return;

   auto* gridController = mGridControlTarget->GetGridController();

   for (int x = 0; x < GetGridControllerCols(); ++x)
   {
      for (int y = 0; y < GetGridControllerRows(); ++y)
      {
         if (gridController->IsMultisliderGrid())
         {
            Vec2i gridPos = ControllerToGrid(Vec2i(x, y));
            gridController->SetLightDirect(x, y, (int)(mGrid->GetVal(gridPos.x, gridPos.y) * 127), force);
         }
         else
         {
            GridColor color = GetGridColor(x, y);
            gridController->SetLight(x, y, color, force);
         }
      }
   }
}

GridColor StepSequencer::GetGridColor(int x, int y)
{
   Vec2i gridPos = ControllerToGrid(Vec2i(x, y));
   bool cellOn = mGrid->GetVal(gridPos.x, gridPos.y) > 0;
   bool cellBright = mGrid->GetVal(gridPos.x, gridPos.y) > kMidwayVelocity;
   bool colOn = (mGrid->GetHighlightCol(gTime) == gridPos.x) && mEnabled;

   GridColor color;
   if (colOn)
   {
      if (cellBright)
         color = kGridColor3Bright;
      else if (cellOn)
         color = kGridColor3Dim;
      else
         color = kGridColor2Dim;
   }
   else
   {
      if (cellBright)
         color = kGridColor1Bright;
      else if (cellOn)
         color = kGridColor1Dim;
      else
         color = kGridColorOff;
   }

   return color;
}

void StepSequencer::UpdateVelocityLights()
{
   if (mVelocityGridController->GetGridController() == nullptr)
      return;

   float stepVelocity = 0;
   if (mHeldButtons.size() > 0)
      stepVelocity = mGrid->GetVal(mHeldButtons.begin()->mCol, mHeldButtons.begin()->mRow);

   for (int x = 0; x < mVelocityGridController->GetGridController()->NumCols(); ++x)
   {
      for (int y = 0; y < mVelocityGridController->GetGridController()->NumRows(); ++y)
      {
         GridColor color;
         if (stepVelocity >= (8 - y) / 8.0f)
            color = kGridColor2Bright;
         else
            color = kGridColorOff;

         mVelocityGridController->GetGridController()->SetLight(x, y, color);
      }
   }
}

void StepSequencer::UpdateMetaLights()
{
   if (mMetaStepGridController->GetGridController() == nullptr)
      return;

   bool hasHeldButtons = mHeldButtons.size() > 0;
   juce::uint32 metaStepMask = 0;
   if (hasHeldButtons)
      metaStepMask = mMetaStepMasks[GetMetaStepMaskIndex(mHeldButtons.begin()->mCol, mHeldButtons.begin()->mRow)];

   for (int x = 0; x < mMetaStepGridController->GetGridController()->NumCols(); ++x)
   {
      for (int y = 0; y < mMetaStepGridController->GetGridController()->NumRows(); ++y)
      {
         GridColor color;
         if (hasHeldButtons && (metaStepMask & (1 << x)))
            color = kGridColor1Bright;
         else if (!hasHeldButtons && x == GetMetaStep(gTime))
            color = kGridColor3Bright;
         else
            color = kGridColorOff;

         mMetaStepGridController->GetGridController()->SetLight(x, y, color);
      }
   }
}

void StepSequencer::OnControllerPageSelected()
{
   UpdateLights(true);
}

void StepSequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (mPush2Connected || grid == mGridControlTarget->GetGridController())
   {
      bool press = velocity > 0;
      if (x >= 0 && y >= 0)
      {
         Vec2i gridPos = ControllerToGrid(Vec2i(x, y));

         if (grid != nullptr && grid->IsMultisliderGrid())
         {
            mGrid->SetVal(gridPos.x, gridPos.y, velocity);
         }
         else
         {
            if (press)
            {
               mHeldButtons.push_back(HeldButton(gridPos.x, gridPos.y));
               float val = mGrid->GetVal(gridPos.x, gridPos.y);
               if (val == 0)
               {
                  float strength = mStrength;
                  mGrid->SetVal(gridPos.x, gridPos.y, strength);
                  mHeldButtons.rbegin()->mTime = 0;
               }
            }
            else
            {
               double holdTime = 0;
               for (auto iter = mHeldButtons.begin(); iter != mHeldButtons.end(); ++iter)
               {
                  if (iter->mCol == gridPos.x && iter->mRow == gridPos.y)
                  {
                     holdTime = gTime - iter->mTime;
                     mHeldButtons.erase(iter);
                     break;
                  }
               }

               if (holdTime < 500)
               {
                  float val = mGrid->GetVal(gridPos.x, gridPos.y);
                  if (val == mStrength)
                     mGrid->SetVal(gridPos.x, gridPos.y, 0);
                  else
                     mGrid->SetVal(gridPos.x, gridPos.y, mStrength);
               }
            }
         }

         UpdateLights();
         UpdateVelocityLights();
         UpdateMetaLights();
      }
   }
   else if (grid == mVelocityGridController->GetGridController())
   {
      if (velocity > 0)
      {
         for (auto iter : mHeldButtons)
         {
            mGrid->SetVal(iter.mCol, iter.mRow, (8 - y) / 8.0f);
            iter.mTime = 0;
         }
         UpdateVelocityLights();
      }
   }
   else if (grid == mMetaStepGridController->GetGridController())
   {
      if (velocity > 0)
      {
         for (auto iter : mHeldButtons)
         {
            mMetaStepMasks[GetMetaStepMaskIndex(iter.mCol, iter.mRow)] ^= 1 << x;
            iter.mTime = 0;
         }
         UpdateMetaLights();
      }
   }
}

int StepSequencer::GetStep(int step, int pitch)
{
   return ofClamp(mGrid->GetVal(step, pitch), 0, 1) * 127;
}

void StepSequencer::SetStep(int step, int pitch, int velocity)
{
   mGrid->SetVal(step, pitch, ofClamp(velocity / 127.0f, 0, 1));
   UpdateLights();
}

int StepSequencer::GetGridControllerRows()
{
   if (mGridControlTarget->GetGridController())
      return mGridControlTarget->GetGridController()->NumRows();
   if (mPush2Connected)
      return 8;
   return 8;
}

int StepSequencer::GetGridControllerCols()
{
   if (mGridControlTarget->GetGridController())
      return mGridControlTarget->GetGridController()->NumCols();
   if (mPush2Connected)
      return 8;
   return 8;
}

Vec2i StepSequencer::ControllerToGrid(const Vec2i& controller)
{
   if (!HasGridController())
      return Vec2i(0, 0);

   int rows = GetGridControllerRows();
   int cols = GetGridControllerCols();

   int numChunks = GetNumControllerChunks();
   int chunkSize = mGrid->GetRows() / numChunks;
   int col = controller.x + (controller.y / chunkSize) * cols;
   int row = (chunkSize - 1) - (controller.y % chunkSize) + mGridYOff * chunkSize;
   return Vec2i(col, row);
}

int StepSequencer::GetNumControllerChunks()
{
   if (!HasGridController())
      return 1;

   int rows = GetGridControllerRows();
   int cols = GetGridControllerCols();

   int numBreaks = int((mGrid->GetCols() / MAX(1.0f, cols)) + .5f);
   int numChunks = int(mGrid->GetRows() / MAX(1.0f, (rows / MAX(1, numBreaks))) + .5f);
   return numChunks;
}

void StepSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGridYOffDropdown->SetShowing(HasGridController());
   mAdjustOffsetsCheckbox->SetShowing(!mHasExternalPulseSource);
   mCurrentColumnSlider->SetExtents(0, GetNumSteps(mStepInterval, mNumMeasures));
   mRepeatRateDropdown->SetShowing(mNoteInputMode == NoteInputMode::RepeatHeld);

   mGrid->Draw();
   mStrengthSlider->Draw();
   mNumMeasuresSlider->Draw();
   mPresetDropdown->Draw();
   mAdjustOffsetsCheckbox->Draw();
   mRepeatRateDropdown->Draw();
   mGridYOffDropdown->Draw();
   mStepIntervalDropdown->Draw();
   mShiftLeftButton->Draw();
   mShiftRightButton->Draw();
   mGridControlTarget->Draw();
   mVelocityGridController->Draw();
   mMetaStepGridController->Draw();
   mRandomizationAmountSlider->Draw();
   mRandomizationDensitySlider->Draw();
   mRandomizeButton->Draw();

   float gridX, gridY;
   mGrid->GetPosition(gridX, gridY, true);
   for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
   {
      if (i < mNumRows)
      {
         float y = gridY + mGrid->GetHeight() - (i + 1) * (mGrid->GetHeight() / float(mNumRows));

         if (mAdjustOffsets)
         {
            mOffsetSlider[i]->SetShowing(true);
            mOffsetSlider[i]->SetPosition(gridX + mGrid->GetWidth() + 5, y);
            mOffsetSlider[i]->Draw();
         }
         else
         {
            mOffsetSlider[i]->SetShowing(false);
         }

         mRandomizeRowButton[i]->Draw();

         mRows[i]->Draw(gridX, y);
      }
      else
      {
         mOffsetSlider[i]->SetShowing(false);
      }
   }

   if (HasGridController())
   {
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(4);
      ofSetColor(255, 0, 0, 50);
      float squareh = float(mGrid->GetHeight()) / mNumRows;
      float squarew = float(mGrid->GetWidth()) / GetNumSteps(mStepInterval, mNumMeasures);
      int chunkSize = mGrid->GetRows() / GetNumControllerChunks();
      float width = MIN(mGrid->GetWidth(), squarew * GetGridControllerCols() * GetNumControllerChunks());
      ofRect(gridX, gridY + squareh * (mNumRows - chunkSize) - squareh * mGridYOff * chunkSize, width, squareh * chunkSize);
      ofPopStyle();
   }

   ofPushStyle();
   ofFill();
   for (int col = 0; col < mGrid->GetCols(); ++col)
   {
      for (int row = 0; row < mGrid->GetRows(); ++row)
      {
         auto mask = mMetaStepMasks[GetMetaStepMaskIndex(col, row)];
         ofVec2f pos = mGrid->GetCellPosition(col, row) + mGrid->GetPosition(true);
         float cellWidth = (float)mGrid->GetWidth() / mGrid->GetCols();
         float cellHeight = (float)mGrid->GetHeight() / mGrid->GetRows();
         for (int i = 0; i < kMetaStepLoop; ++i)
         {
            if (mask != 0xff)
            {
               float x = pos.x + ((i % 4) + 1.5f) * (cellWidth / 6);
               float y = pos.y + ((i / 4 + 1.5f) * (cellHeight / 4)) - cellHeight;
               float radius = cellHeight * .08f;

               if (i == GetMetaStep(gTime))
               {
                  ofSetColor(255, 220, 0);
                  ofCircle(x, y, radius * 1.5f);
               }

               if ((mask & (1 << i)) == 0)
                  ofSetColor(0, 0, 0);
               else
                  ofSetColor(255, 0, 0);

               ofCircle(x, y, radius);
            }
         }
      }
   }
   ofPopStyle();
}

void StepSequencer::DrawRowLabel(const char* label, int row, int x, int y)
{
   DrawTextRightJustify(label, x, y + row * 9.4f);
}

void StepSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + 45;
   if (mAdjustOffsets)
      width += 100;
   height = mGrid->GetHeight() + 50;
}

void StepSequencer::Resize(float w, float h)
{
   float extraW = 45;
   float extraH = 50;
   if (mAdjustOffsets)
      extraW += 100;
   mGrid->SetDimensions(MAX(w - extraW, 185), MAX(h - extraH, 46 + 13 * mNumRows));
}

void StepSequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   if (mGrid->TestClick(x, y, right))
      UpdateLights();
}

void StepSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
   UpdateLights();
}

bool StepSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   if (mGrid->NotifyMouseMoved(x, y))
      UpdateLights();
   return false;
}

bool StepSequencer::OnPush2Control(MidiMessageType type, int controlIndex, float midiValue)
{
   mPush2Connected = true;

   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int gridX = gridIndex % 8;
         int gridY = 7 - gridIndex / 8;
         OnGridButton(gridX, gridY, midiValue / 127, mGridControlTarget->GetGridController());
         return true;
      }
   }

   if (type == kMidiMessage_PitchBend)
   {
      if (midiValue != 8192) //default value, happens on pitch bend release
      {
         float val = midiValue / 16320.0f;
         float oldStrength = mStrength;
         mStrength = val;
         FloatSliderUpdated(mStrengthSlider, oldStrength);
      }
      else
      {
         mStrength = 1;
      }
      return true;
   }

   return false;
}

void StepSequencer::UpdatePush2Leds(Push2Control* push2)
{
   mPush2Connected = true;

   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         GridColor color = GetGridColor(x, y);
         int pushColor = 0;
         switch (color)
         {
            case kGridColorOff: //off
               pushColor = 0;
               break;
            case kGridColor1Dim: //
               pushColor = 86;
               break;
            case kGridColor1Bright: //pressed
               pushColor = 32;
               break;
            case kGridColor2Dim:
               pushColor = 114;
               break;
            case kGridColor2Bright: //root
               pushColor = 25;
               break;
            case kGridColor3Dim: //not in pentatonic
               pushColor = 116;
               break;
            case kGridColor3Bright: //in pentatonic
               pushColor = 115;
               break;
         }
         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

int StepSequencer::GetNumSteps(NoteInterval interval, int numMeasures) const
{
   return TheTransport->CountInStandardMeasure(interval) * TheTransport->GetTimeSigTop() / TheTransport->GetTimeSigBottom() * numMeasures;
}

int StepSequencer::GetStepNum(double time)
{
   int measure = TheTransport->GetMeasure(time) % mNumMeasures;
   int stepsPerMeasure = GetNumSteps(mStepInterval, mNumMeasures);

   return TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure + measure * stepsPerMeasure / mNumMeasures;
}

void StepSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void StepSequencer::Step(double time, float velocity, int pulseFlags)
{
   if (!mIsSetUp)
      return;

   mGrid->SetGrid(GetNumSteps(mStepInterval, mNumMeasures), mNumRows);

   if (!mEnabled)
   {
      UpdateLights();
      return;
   }

   int direction = 1;
   if (pulseFlags & kPulseFlag_Backward)
      direction = -1;
   if (pulseFlags & kPulseFlag_Repeat)
      direction = 0;

   mCurrentColumn = (mCurrentColumn + direction + GetNumSteps(mStepInterval, mNumMeasures)) % GetNumSteps(mStepInterval, mNumMeasures);

   if (pulseFlags & kPulseFlag_Reset)
      mCurrentColumn = 0;
   else if (pulseFlags & kPulseFlag_Random)
      mCurrentColumn = gRandom() % GetNumSteps(mStepInterval, mNumMeasures);

   if (!mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport))
      mCurrentColumn = GetStepNum(time);

   if (pulseFlags & kPulseFlag_Align)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int numMeasures = ceil(float(GetNumSteps(mStepInterval, mNumMeasures)) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      int step = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % GetNumSteps(mStepInterval, mNumMeasures);
      mCurrentColumn = step;
   }

   mGrid->SetHighlightCol(time, mCurrentColumn);
   UpdateLights();
   UpdateMetaLights();

   if (mHasExternalPulseSource)
   {
      for (auto& row : mRows)
         row->PlayStep(time, mCurrentColumn);
   }
}

void StepSequencer::PlayStepNote(double time, int note, float val)
{
   mNoteOutput.PlayNote(time, note, val * 127);
}

void StepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mNoteInputMode == NoteInputMode::PlayStepIndex)
   {
      if (velocity > 0)
      {
         if (!mHasExternalPulseSource)
         {
            mHasExternalPulseSource = true;
            mAdjustOffsets = false;
            for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
               mOffsets[i] = 0;
         }

         mCurrentColumn = pitch % GetNumSteps(mStepInterval, mNumMeasures);
         Step(time, velocity / 127.0f, kPulseFlag_Repeat);
      }
   }
   else
   {
      if (mRepeatRate == kInterval_None)
         PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      else
         mPadPressures[pitch] = velocity;
   }
}

void StepSequencer::OnPulse(double time, float velocity, int flags)
{
   if (!mHasExternalPulseSource)
   {
      mHasExternalPulseSource = true;
      mAdjustOffsets = false;
      for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
         mOffsets[i] = 0;
   }

   Step(time, velocity, flags);
}


void StepSequencer::SendPressure(int pitch, int pressure)
{
   mPadPressures[pitch] = pressure;
}

void StepSequencer::Exit()
{
   IDrawableModule::Exit();
   if (mGridControlTarget->GetGridController() != nullptr && !mGridControlTarget->GetGridController()->IsConnected())
      mGridControlTarget->GetGridController()->ResetLights();
}

void StepSequencer::SetPreset(int preset)
{
   mPreset = preset;

   mGrid->Clear();

   switch (preset)
   {
      case 0:
         for (int i = 0; i < 16; ++i)
            mGrid->SetValRefactor(2, i, 1.0f);
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(0, 8, 1.0f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         break;
      case 1:
         for (int i = 0; i < 16; i += 2)
            mGrid->SetValRefactor(2, i, 1.0f);
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(0, 8, 1.0f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         break;
      case 2:
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(0, 4, 1.0f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(0, 8, 1.0f);
         mGrid->SetValRefactor(0, 12, 1.0f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         break;
      case 3: //amen
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(6, 0, .25f);
         mGrid->SetValRefactor(0, 2, 1.0f);
         mGrid->SetValRefactor(6, 2, .25f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(2, 6, 1.0f);
         mGrid->SetValRefactor(6, 6, .25f);
         mGrid->SetValRefactor(4, 7, .5f);
         mGrid->SetValRefactor(2, 8, 1.0f);
         mGrid->SetValRefactor(6, 8, .25f);
         mGrid->SetValRefactor(4, 9, .5f);
         mGrid->SetValRefactor(0, 10, 1.0f);
         mGrid->SetValRefactor(6, 10, .25f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         mGrid->SetValRefactor(2, 14, 1.0f);
         mGrid->SetValRefactor(6, 15, .25f);
         break;
      case 4:
         for (int i = 0; i < 16; i += 2)
            mGrid->SetValRefactor(2, i, 1.0f);
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(1, 7, 1.0f);
         mGrid->SetValRefactor(1, 9, 1.0f);
         mGrid->SetValRefactor(0, 10, 1.0f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         break;
      case 5:
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(1, 8, 1.0f);
         break;
      case 6:
         mGrid->SetValRefactor(0, 0, 1.0f);
         mGrid->SetValRefactor(2, 2, 1.0f);
         mGrid->SetValRefactor(0, 4, 1.0f);
         mGrid->SetValRefactor(1, 4, 1.0f);
         mGrid->SetValRefactor(2, 6, 1.0f);
         mGrid->SetValRefactor(0, 8, 1.0f);
         mGrid->SetValRefactor(2, 10, 1.0f);
         mGrid->SetValRefactor(0, 12, 1.0f);
         mGrid->SetValRefactor(1, 12, 1.0f);
         mGrid->SetValRefactor(2, 14, 1.0f);
         break;
   }
}

int StepSequencer::GetMetaStep(double time)
{
   return TheTransport->GetMeasure(time) % kMetaStepLoop;
}

bool StepSequencer::IsMetaStepActive(double time, int col, int row)
{
   return mMetaStepMasks[GetMetaStepMaskIndex(col, row)] & (1 << GetMetaStep(time));
}

bool StepSequencer::HasGridController()
{
   if (mPush2Connected)
      return true;
   return mGridControlTarget->GetGridController() != nullptr && mGridControlTarget->GetGridController()->IsConnected();
}

void StepSequencer::RandomizeRow(int row)
{
   for (int col = 0; col < mGrid->GetCols(); ++col)
   {
      if (ofRandom(1) < mRandomizationAmount)
      {
         float value = 0;
         if (ofRandom(1) < mRandomizationDensity)
            value = ofRandom(1);
         mGrid->SetVal(col, row, value);
      }
   }
}

void StepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void StepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mStrengthSlider)
   {
      mGrid->SetStrength(mStrength);
      for (auto iter : mHeldButtons)
         mGrid->SetVal(iter.mCol, iter.mRow, mStrength);

      if (mHeldButtons.size() > 0)
         UpdateLights();
   }
   for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
   {
      if (slider == mOffsetSlider[i])
      {
         float offset = -mOffsets[i] / 32; //up to 1/32nd late or early
         mRows[i]->SetOffset(offset);
         mNoteRepeats[i]->SetOffset(offset);
         mGrid->SetDrawOffset(i, mOffsets[i] / 2);
      }
   }
}

void StepSequencer::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void StepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumMeasuresSlider)
   {
      mGrid->SetGrid(GetNumSteps(mStepInterval, mNumMeasures), mNumRows);
      if (mNumMeasures > oldVal)
      {
         int newChunkCount = ceil(float(mNumMeasures) / oldVal);
         int stepsPerChunk = GetNumSteps(mStepInterval, oldVal);
         for (int chunk = 1; chunk < newChunkCount; ++chunk)
         {
            for (int col = 0; col < stepsPerChunk && col + stepsPerChunk * chunk < mGrid->GetCols(); ++col)
            {
               for (int row = 0; row < mGrid->GetRows(); ++row)
               {
                  mGrid->SetVal(col + stepsPerChunk * chunk, row, mGrid->GetVal(col, row), false);
               }
            }
         }
      }
   }
}

void StepSequencer::ButtonClicked(ClickButton* button)
{
   if (button == mShiftLeftButton || button == mShiftRightButton)
   {
      int shift = (button == mShiftRightButton) ? 1 : -1;

      for (int row = 0; row < mGrid->GetRows(); ++row)
      {
         int start = (shift == 1) ? mGrid->GetCols() - 1 : 0;
         int end = (shift == 1) ? 0 : mGrid->GetCols() - 1;
         float startVal = mGrid->GetVal(start, row);
         for (int col = start; col != end; col -= shift)
            mGrid->SetVal(col, row, mGrid->GetVal(col - shift, row));
         mGrid->SetVal(end, row, startVal);
      }
   }
   if (button == mRandomizeButton)
   {
      for (int row = 0; row < mGrid->GetRows(); ++row)
         RandomizeRow(row);
   }
   for (int row = 0; row < mRandomizeRowButton.size(); ++row)
   {
      if (button == mRandomizeRowButton[row])
         RandomizeRow(row);
   }
}

void StepSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mPresetDropdown)
      SetPreset(mPreset);
   if (list == mRepeatRateDropdown)
   {
      for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
         mNoteRepeats[i]->SetInterval(mRepeatRate);
   }
   if (list == mStepIntervalDropdown)
   {
      UIGrid* oldGrid = new UIGrid(*mGrid);
      int oldNumSteps = GetNumSteps((NoteInterval)oldVal, mNumMeasures);
      int newNumSteps = GetNumSteps(mStepInterval, mNumMeasures);
      for (int i = 0; i < mGrid->GetRows(); ++i)
      {
         for (int j = 0; j < newNumSteps; ++j)
         {
            float div = j * ((float)oldNumSteps / newNumSteps);
            int col = (int)div;
            if (div == col)
               mGrid->SetValRefactor(i, j, oldGrid->GetValRefactor(i, col));
            else
               mGrid->SetValRefactor(i, j, 0);
         }
      }
      oldGrid->Delete();
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mStepInterval;
      mFlusher.SetInterval(mStepInterval);
      mGrid->SetMajorColSize(TheTransport->CountInStandardMeasure(mStepInterval) / 4);
      for (int i = 0; i < NUM_STEPSEQ_ROWS; ++i)
         mRows[i]->UpdateTimeListener();
   }
}

void StepSequencer::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);

   ofVec2f mousePos(TheSynth->GetMouseX(GetOwningContainer()), TheSynth->GetMouseY(GetOwningContainer()));
   if (key >= '1' && key <= '8' && mGrid->GetRect().contains(mousePos.x, mousePos.y))
   {
      int metaStep = key - '1';
      auto cell = mGrid->GetGridCellAt(mousePos.x - mGrid->GetPosition().x, mousePos.y - mGrid->GetPosition().y);
      mMetaStepMasks[GetMetaStepMaskIndex(cell.mCol, cell.mRow)] ^= (1 << metaStep);
   }
}

void StepSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void StepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("gridrows", moduleInfo, 8, 1, NUM_STEPSEQ_ROWS);
   mModuleSaveData.LoadInt("gridmeasures", moduleInfo, 1, 1, 16);
   mModuleSaveData.LoadBool("multislider_mode", moduleInfo, true);

   EnumMap noteInputModeMap;
   noteInputModeMap["play step index"] = (int)NoteInputMode::PlayStepIndex;
   noteInputModeMap["repeat held"] = (int)NoteInputMode::RepeatHeld;
   mModuleSaveData.LoadEnum<NoteInputMode>("note_input_mode", moduleInfo, (int)NoteInputMode::PlayStepIndex, nullptr, &noteInputModeMap);

   SetUpFromSaveData();
}

void StepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mNumRows = mModuleSaveData.GetInt("gridrows");
   mGrid->SetGrid(GetNumSteps(mStepInterval, mNumMeasures), mNumRows);

   bool multisliderMode = mModuleSaveData.GetBool("multislider_mode");
   mGrid->SetGridMode(multisliderMode ? UIGrid::kMultisliderBipolar : UIGrid::kNormal);
   mGrid->SetRestrictDragToRow(multisliderMode);
   mGrid->SetRequireShiftForMultislider(true);

   mNoteInputMode = mModuleSaveData.GetEnum<NoteInputMode>("note_input_mode");

   if (mNoteInputMode == NoteInputMode::RepeatHeld)
      mHasExternalPulseSource = false;

   mIsSetUp = true;
}

void StepSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mGrid->SaveState(out);

   int numMetaStepMasks = META_STEP_MAX * NUM_STEPSEQ_ROWS;
   out << numMetaStepMasks;
   for (int i = 0; i < numMetaStepMasks; ++i)
      out << mMetaStepMasks[i];
   out << mHasExternalPulseSource;

   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
}

void StepSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mGrid->LoadState(in);

   if (rev >= 1)
   {
      int numMetaStepMasks;
      in >> numMetaStepMasks;
      for (int i = 0; i < numMetaStepMasks; ++i)
         in >> mMetaStepMasks[i];
   }
   if (rev >= 2)
      in >> mHasExternalPulseSource;
   if (rev >= 3)
   {
      float gridWidth, gridHeight;
      in >> gridWidth;
      in >> gridHeight;
      mGrid->SetDimensions(gridWidth, gridHeight);
   }
}

StepSequencerRow::StepSequencerRow(StepSequencer* seq, UIGrid* grid, int row)
: mSeq(seq)
, mGrid(grid)
, mRow(row)
{
   mRowPitch = row;
   TheTransport->AddListener(this, mSeq->GetStepInterval(), OffsetInfo(0, true), true);
}

StepSequencerRow::~StepSequencerRow()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerRow::CreateUIControls()
{
   mRowPitchEntry = new TextEntry(mSeq, ("rowpitch" + ofToString(mRow)).c_str(), -1, -1, 3, &mRowPitch, 0, 127);
}

void StepSequencerRow::OnTimeEvent(double time)
{
   if (mSeq->Enabled() == false || mSeq->HasExternalPulseSource())
      return;

   float offsetMs = mOffset * TheTransport->MsPerBar();
   int step = mSeq->GetStepNum(time + offsetMs);
   PlayStep(time, step);
}

void StepSequencerRow::PlayStep(double time, int step)
{
   float val = mGrid->GetVal(step, mRow);
   if (val > 0 && mSeq->IsMetaStepActive(time, step, mRow))
   {
      mSeq->PlayStepNote(time, mRowPitch, val * val);
      mPlayedSteps[mPlayedStepsRoundRobin].step = step;
      mPlayedSteps[mPlayedStepsRoundRobin].time = time;
      mPlayedStepsRoundRobin = (mPlayedStepsRoundRobin + 1) % mPlayedSteps.size();
   }
}

void StepSequencerRow::SetOffset(float offset)
{
   mOffset = offset;
   UpdateTimeListener();
}

void StepSequencerRow::UpdateTimeListener()
{
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
   {
      transportListenerInfo->mInterval = mSeq->GetStepInterval();
      transportListenerInfo->mOffsetInfo = OffsetInfo(mOffset, false);
   }
}

void StepSequencerRow::Draw(float x, float y)
{
   float xCellSize = float(mGrid->GetWidth()) / mGrid->GetCols();
   float yCellSize = float(mGrid->GetHeight()) / mGrid->GetRows();

   bool showTextEntry = yCellSize > 14;
   mRowPitchEntry->SetShowing(showTextEntry);
   mRowPitchEntry->SetPosition(x - 32, y);
   mRowPitchEntry->Draw();

   if (!showTextEntry)
      DrawTextRightJustify(ofToString(mRowPitch), x - 7, y + 10);

   const float kPlayHighlightDurationMs = 250;
   for (size_t i = 0; i < mPlayedSteps.size(); ++i)
   {
      if (mPlayedSteps[i].time != -1)
      {
         if (gTime - mPlayedSteps[i].time < kPlayHighlightDurationMs)
         {
            if (gTime - mPlayedSteps[i].time > 0)
            {
               float fade = (1 - (gTime - mPlayedSteps[i].time) / kPlayHighlightDurationMs);
               ofPushStyle();
               ofSetLineWidth(3 * fade);
               ofVec2f pos = mGrid->GetCellPosition(mPlayedSteps[i].step, mRow) + mGrid->GetPosition(true);
               ofSetColor(ofColor::white, fade * 255);
               ofRect(pos.x, pos.y, xCellSize, yCellSize);
               ofPopStyle();
            }
         }
         else
         {
            mPlayedSteps[i].time = -1;
         }
      }
   }
}

NoteRepeat::NoteRepeat(StepSequencer* seq, int row)
: mSeq(seq)
, mRow(row)
, mOffset(0)
, mInterval(kInterval_None)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

NoteRepeat::~NoteRepeat()
{
   TheTransport->RemoveListener(this);
}

void NoteRepeat::OnTimeEvent(double time)
{
   int pressure = mSeq->GetPadPressure(mRow);
   if (pressure > 10)
      mSeq->PlayStepNote(time, mSeq->GetRowPitch(mRow), pressure / 85.0f);
}

void NoteRepeat::SetInterval(NoteInterval interval)
{
   mInterval = interval;
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
   {
      transportListenerInfo->mInterval = mInterval;
      transportListenerInfo->mOffsetInfo = OffsetInfo(mOffset, false);
   }
}

void NoteRepeat::SetOffset(float offset)
{
   mOffset = offset;
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
   {
      transportListenerInfo->mInterval = mInterval;
      transportListenerInfo->mOffsetInfo = OffsetInfo(mOffset, false);
   }
}

StepSequencerNoteFlusher::StepSequencerNoteFlusher(StepSequencer* seq)
: mSeq(seq)
{
   TheTransport->AddListener(this, mSeq->GetStepInterval(), OffsetInfo(.01f, false), true);
}

StepSequencerNoteFlusher::~StepSequencerNoteFlusher()
{
   TheTransport->RemoveListener(this);
}

void StepSequencerNoteFlusher::SetInterval(NoteInterval interval)
{
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
   {
      transportListenerInfo->mInterval = interval;
      transportListenerInfo->mOffsetInfo = OffsetInfo(.01f, false);
   }
}

void StepSequencerNoteFlusher::OnTimeEvent(double time)
{
   mSeq->Flush(time);
}

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
//  NoteStepSequencer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/3/13.
//
//

#include "NoteStepSequencer.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "NoteStepSequencer.h"
#include "LaunchpadInterpreter.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"
#include "MathUtils.h"

NoteStepSequencer::NoteStepSequencer()
{
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      mVels[i] = ofRandom(1) < .5f ? 127 : 0;
      mNoteLengths[i] = ofRandom(1) < .5f ? .5f : 1;
   }

   RandomizePitches(false);

   TheScale->AddListener(this);
}

void NoteStepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(130);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(4)
   BUTTON(mRandomizeAllButton, "random");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mRandomizePitchButton, "pitch");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mRandomizeVelocityButton, "vel");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mRandomizeLengthButton, "len");
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(8);
   UICONTROL_CUSTOM(mGridControlTarget, new GridControlTarget(UICONTROL_BASICS("grid")));
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetXSlider, "x offset", &mGridControlOffsetX, 0, 16);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetYSlider, "y offset", &mGridControlOffsetY, 0, 16);
   UIBLOCK_NEWLINE();
   UIBLOCK_PUSHSLIDERWIDTH(150);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, NSS_MAX_STEPS);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShiftBackButton, "<");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShiftForwardButton, ">");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mClearButton, "clear");
   UIBLOCK_NEWLINE();
   UIBLOCK_POPSLIDERWIDTH();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, 0, 7);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNoteModeSelector, "notemode", (int*)(&mNoteMode), 80);
   UIBLOCK_NEWLINE();
   ENDUIBLOCK0();

   UIBLOCK(220, 20, 150);
   FLOATSLIDER(mRandomizePitchChanceSlider, "rand pitch chance", &mRandomizePitchChance, 0, 1);
   INTSLIDER(mRandomizePitchVarietySlider, "rand pitch variety", &mRandomizePitchVariety, 1, 10);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER(mRandomizeLengthChanceSlider, "rand len chance", &mRandomizeLengthChance, 0, 1);
   FLOATSLIDER(mRandomizeLengthRangeSlider, "rand len range", &mRandomizeLengthRange, 0, 1);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER(mRandomizeVelocityChanceSlider, "rand vel chance", &mRandomizeVelocityChance, 0, 1);
   FLOATSLIDER(mRandomizeVelocityDensitySlider, "rand vel density", &mRandomizeVelocityDensity, 0, 1);
   ENDUIBLOCK0();

   mGrid = new UIGrid("notegrid", 5, 55, 210, 110, 8, 24, this);
   mVelocityGrid = new UIGrid("velocitygrid", 5, 147, 200, 45, 8, 1, this);
   mLoopResetPointSlider = new IntSlider(this, "loop reset", -1, -1, 100, 15, &mLoopResetPoint, 0, mLength);
   mGrid->SetClickValueSubdivisions(mStepLengthSubdivisions);

   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      mToneDropdowns[i] = new DropdownList(this, ("tone" + ofToString(i)).c_str(), -1, -1, &(mTones[i]), 40);
      mToneDropdowns[i]->SetDrawTriangle(false);
      mToneDropdowns[i]->SetShowing(false);
      mVelocitySliders[i] = new IntSlider(this, ("vel" + ofToString(i)).c_str(), -1, -1, 30, 15, &mVels[i], 0, 127);
      mVelocitySliders[i]->SetShowName(false);
      mVelocitySliders[i]->SetShowing(false);
      mLengthSliders[i] = new FloatSlider(this, ("len" + ofToString(i)).c_str(), -1, -1, 30, 15, &mNoteLengths[i], 0.01f, 1, 1);
      mLengthSliders[i]->SetShowName(false);
      mLengthSliders[i]->SetShowing(false);
   }
   SetUpStepControls();

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("none", kInterval_None);

   mNoteModeSelector->AddLabel("scale", kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("pentatonic", kNoteMode_Pentatonic);
   mNoteModeSelector->AddLabel("5ths", kNoteMode_Fifths);

   mGrid->SetSingleColumnMode(true);
   mGrid->SetFlip(true);
   mGrid->SetListener(this);
   mGrid->SetGridMode(UIGrid::kHorislider);
   mGrid->SetRequireShiftForMultislider(true);
   mVelocityGrid->SetGridMode(UIGrid::kMultisliderBipolar);
   mVelocityGrid->SetListener(this);

   mLoopResetPointSlider->SetShowing(mHasExternalPulseSource);

   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      mStepCables[i] = new AdditionalNoteCable();
      mStepCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mStepCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
      AddPatchCableSource(mStepCables[i]->GetPatchCableSource());
   }
}

NoteStepSequencer::~NoteStepSequencer()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
   TheScale->RemoveListener(this);
}

void NoteStepSequencer::SetMidiController(std::string name)
{
   if (mController)
      mController->RemoveListener(this);

   mController = TheSynth->FindMidiController(name);
   if (mController)
      mController->AddListener(this, 0);

   UpdateLights();
}

void NoteStepSequencer::Init()
{
   IDrawableModule::Init();

   SyncGridToSeq();
   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheTransport->AddAudioPoller(this);
}

void NoteStepSequencer::Poll()
{
   if (mGridSyncQueued)
   {
      SyncGridToSeq();
      mGridSyncQueued = false;
   }
   UpdateGridControllerLights(false);
}

void NoteStepSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   mLoopResetPointSlider->SetShowing(mHasExternalPulseSource);
   mGridControlOffsetXSlider->SetShowing(mGridControlTarget->GetGridController() != nullptr && mLength > mGridControlTarget->GetGridController()->NumCols());
   mGridControlOffsetYSlider->SetShowing(mGridControlTarget->GetGridController() != nullptr && mNoteRange > mGridControlTarget->GetGridController()->NumRows());

   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mOctaveSlider->Draw();
   mNoteModeSelector->Draw();
   mShiftBackButton->Draw();
   mShiftForwardButton->Draw();
   mClearButton->Draw();
   mRandomizeAllButton->Draw();
   mRandomizePitchButton->Draw();
   mRandomizeLengthButton->Draw();
   mRandomizeVelocityButton->Draw();
   mLoopResetPointSlider->Draw();
   mGridControlTarget->Draw();
   mGridControlOffsetXSlider->Draw();
   mGridControlOffsetYSlider->Draw();
   mRandomizePitchChanceSlider->Draw();
   mRandomizePitchVarietySlider->Draw();
   mRandomizeLengthChanceSlider->Draw();
   mRandomizeLengthRangeSlider->Draw();
   mRandomizeVelocityChanceSlider->Draw();
   mRandomizeVelocityDensitySlider->Draw();

   mGrid->Draw();
   mVelocityGrid->Draw();

   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i = 0; i < mGrid->GetRows(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(0, i - 1) + mGrid->GetPosition(true);
      float scale = MIN(mGrid->IClickable::GetDimensions().y / mGrid->GetRows(), 20);
      DrawTextNormal(NoteName(RowToPitch(i), false, true) + "(" + ofToString(RowToPitch(i)) + ")", pos.x + 1, pos.y - (scale / 8), scale);
   }
   ofPopStyle();

   if (mGridControlTarget->GetGridController())
   {
      int controllerCols = mGridControlTarget->GetGridController()->NumCols();
      int controllerRows = mGridControlTarget->GetGridController()->NumRows();

      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(4);
      ofSetColor(255, 0, 0, 50);
      float squareh = float(mGrid->GetHeight()) / mNoteRange;
      float squarew = float(mGrid->GetWidth()) / mLength;
      ofRectangle gridRect = mGrid->GetRect(K(local));
      ofRect(gridRect.x + squarew * mGridControlOffsetX,
             gridRect.y + gridRect.height - squareh * (mGridControlOffsetY + controllerRows),
             squarew * controllerCols,
             squareh * controllerRows);
      ofPopStyle();
   }

   ofPushStyle();
   ofFill();
   float gridX, gridY, gridW, gridH;
   mGrid->GetPosition(gridX, gridY, true);
   gridW = mGrid->GetWidth();
   gridH = mGrid->GetHeight();
   float boxHeight = (float(gridH) / mNoteRange);
   float boxWidth = (float(gridW) / mGrid->GetCols());

   for (int i = 0; i < mGrid->GetCols() - 1; ++i)
   {
      if (mNoteLengths[i] == 1 && mTones[i] == mTones[i + 1] && mVels[i] > mVels[i + 1] && mVels[i + 1] != 0)
      {
         ofSetColor(255, 255, 255, 255);
         ofFill();
         float y = gridY + gridH - mTones[i] * boxHeight;
         ofRect(gridX + boxWidth * i + 1, y - boxHeight + 1, boxWidth * 1.5f - 2, boxHeight - 2);
      }
   }

   for (int i = 0; i < mNoteRange; ++i)
   {
      if (RowToPitch(i) % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot() % TheScale->GetPitchesPerOctave())
         ofSetColor(0, 255, 0, 80);
      else if (TheScale->GetPitchesPerOctave() == 12 && RowToPitch(i) % TheScale->GetPitchesPerOctave() == (TheScale->ScaleRoot() + 7) % TheScale->GetPitchesPerOctave())
         ofSetColor(200, 150, 0, 80);
      else if (mNoteMode == kNoteMode_Chromatic && TheScale->IsInScale(RowToPitch(i)))
         ofSetColor(100, 75, 0, 80);
      else
         continue;

      float y = gridY + gridH - i * boxHeight;
      ofRect(gridX, y - boxHeight, gridW, boxHeight);
   }

   for (int i = 0; i < mGrid->GetCols(); ++i)
   {
      const float kPlayHighlightDurationMs = 250;
      if (mLastStepPlayTime[i] != -1)
      {
         if (gTime - mLastStepPlayTime[i] < kPlayHighlightDurationMs)
         {
            if (gTime - mLastStepPlayTime[i] > 0)
            {
               float fade = (1 - (gTime - mLastStepPlayTime[i]) / kPlayHighlightDurationMs);
               ofPushStyle();
               ofNoFill();
               ofSetLineWidth(3 * fade);
               ofVec2f pos = mGrid->GetCellPosition(i, mTones[i]) + mGrid->GetPosition(true);
               float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
               float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();
               ofSetColor(ofColor::white, fade * 255);
               ofRect(pos.x, pos.y, xsize, ysize);
               ofPopStyle();
            }
         }
         else
         {
            mLastStepPlayTime[i] = -1;
         }
      }
   }

   float controlYPos = gridY + gridH + mVelocityGrid->GetHeight();
   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   if (mLoopResetPointSlider->IsShowing())
      controlYPos += 19;
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      if (i < mLength)
      {
         mToneDropdowns[i]->SetShowing(mShowStepControls);
         mToneDropdowns[i]->SetPosition(gridX + boxWidth * i, controlYPos);
         mToneDropdowns[i]->SetWidth(std::min(boxWidth, 30.0f));
         mToneDropdowns[i]->Draw();

         mVelocitySliders[i]->SetShowing(mShowStepControls);
         mVelocitySliders[i]->SetPosition(gridX + boxWidth * i, controlYPos + 17);
         mVelocitySliders[i]->SetDimensions(boxWidth, 15);
         mVelocitySliders[i]->Draw();

         mLengthSliders[i]->SetShowing(mShowStepControls);
         mLengthSliders[i]->SetPosition(gridX + boxWidth * i, controlYPos + 32);
         mLengthSliders[i]->SetDimensions(boxWidth, 15);
         mLengthSliders[i]->Draw();
      }
      else
      {
         mToneDropdowns[i]->SetShowing(false);
         mVelocitySliders[i]->SetShowing(false);
         mLengthSliders[i]->SetShowing(false);
      }

      if (i < mLength && mShowStepControls)
      {
         ofVec2f pos = mVelocityGrid->GetCellPosition(i, 0) + mVelocityGrid->GetPosition(true);
         pos.x += mVelocityGrid->GetWidth() / float(mLength) * .5f;
         pos.y = moduleHeight - 7;
         mStepCables[i]->GetPatchCableSource()->SetManualPosition(pos.x, pos.y);
         mStepCables[i]->GetPatchCableSource()->SetEnabled(true);
      }
      else
      {
         mStepCables[i]->GetPatchCableSource()->SetEnabled(false);
      }
   }

   ofPopStyle();
}

void NoteStepSequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mGrid->TestClick(x, y, right);
   mVelocityGrid->TestClick(x, y, right);
}

void NoteStepSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool NoteStepSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool NoteStepSequencer::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mGrid->NotifyMouseScrolled(x, y, scrollX, scrollY, isSmoothScroll, isInvertedScroll);
   mVelocityGrid->NotifyMouseScrolled(x, y, scrollX, scrollY, isSmoothScroll, isInvertedScroll);
   return false;
}

void NoteStepSequencer::SetEnabled(bool on)
{
   mEnabled = on;
   if (!on)
      mNoteOutput.Flush(gTime);
}

void NoteStepSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteStepSequencer::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      for (int i = 0; i < mGrid->GetCols(); ++i)
      {
         bool colHasPitch = false;
         for (int j = 0; j < mGrid->GetRows(); ++j)
         {
            float val = mGrid->GetVal(i, j);
            if (val > 0)
            {
               mTones[i] = j;
               mNoteLengths[i] = val;
               colHasPitch = true;
               break;
            }
         }

         if (!colHasPitch)
            mVels[i] = 0;
         else if (colHasPitch && mVels[i] == 0)
            mVels[i] = 127;
         mVelocityGrid->SetVal(i, 0, mVels[i] / 127.0f, false);
      }
   }
   if (grid == mVelocityGrid)
   {
      for (int i = 0; i < mVelocityGrid->GetCols(); ++i)
         mVels[i] = mVelocityGrid->GetVal(i, 0) * 127;
      SyncGridToSeq();
   }
}

int NoteStepSequencer::RowToPitch(int row)
{
   row += mRowOffset;

   int numPitchesInScale = TheScale->NumTonesInScale();
   switch (mNoteMode)
   {
      case kNoteMode_Scale:
         return TheScale->GetPitchFromTone(row + mOctave * numPitchesInScale + TheScale->GetScaleDegree());
      case kNoteMode_Chromatic:
         return row + mOctave * TheScale->GetPitchesPerOctave();
      case kNoteMode_Pentatonic:
      {
         bool isMinor = TheScale->IsInScale(TheScale->ScaleRoot() + 3);
         const int minorPentatonic[5] = { 0, 3, 5, 7, 10 };
         const int majorPentatonic[5] = { 0, 2, 4, 7, 9 };

         if (isMinor)
            return TheScale->ScaleRoot() + (row / 5 + mOctave) * TheScale->GetPitchesPerOctave() + minorPentatonic[row % 5];
         else
            return TheScale->ScaleRoot() + (row / 5 + mOctave) * TheScale->GetPitchesPerOctave() + majorPentatonic[row % 5];
      }
      case kNoteMode_Fifths:
      {
         int oct = (row / 2) * numPitchesInScale;
         bool isFifth = row % 2 == 1;
         int fifths = oct;
         if (isFifth)
            fifths += 4;
         return TheScale->GetPitchFromTone(fifths + mOctave * numPitchesInScale + TheScale->GetScaleDegree());
      }
   }
   return row;
}

int NoteStepSequencer::PitchToRow(int pitch)
{
   for (int i = 0; i < mGrid->GetRows(); ++i)
   {
      if (pitch == RowToPitch(i))
         return i;
   }
   return -1;
}

void NoteStepSequencer::SetStep(int index, int row, int velocity, float length)
{
   if (index >= 0 && index < NSS_MAX_STEPS)
   {
      mTones[index] = std::clamp(row, 0, MAX_GRID_ROWS - 1);
      mVels[index] = ofClamp(velocity, 0, 127);
      mNoteLengths[index] = length;
      SyncGridToSeq();
   }
}

void NoteStepSequencer::SetPitch(int index, int pitch, int velocity, float length)
{
   if (index >= 0 && index < NSS_MAX_STEPS)
   {
      mTones[index] = PitchToRow(pitch);
      mVels[index] = ofClamp(velocity, 0, 127);
      mNoteLengths[index] = length;
      SyncGridToSeq();
   }
}

void NoteStepSequencer::GetPush2Layout(int& sequenceRows, int& pitchCols, int& pitchRows)
{
   sequenceRows = (mLength - 1) / 8 + 1;
   if (mNoteMode == kNoteMode_Scale && TheScale->NumTonesInScale() == 7)
      pitchCols = 7;
   else
      pitchCols = 8;
   pitchRows = (mNoteRange - 1) / pitchCols + 1;
}

bool NoteStepSequencer::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   int sequenceRows, pitchCols, pitchRows;
   GetPush2Layout(sequenceRows, pitchCols, pitchRows);

   if (type == kMidiMessage_Note)
   {
      if (controlIndex == 12)
      {
         mPush2VelocityHeld = (midiValue > 0);
         return true;
      }

      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;

         if (y < sequenceRows)
         {
            int index = x + y * 8;
            if (midiValue > 0)
            {
               mPush2HeldStep = index;
               if (mVels[mPush2HeldStep] == 0)
               {
                  mVels[mPush2HeldStep] = mQueuedPush2Vel;
                  mPush2HeldStepWasEdited = true;
               }
               else
               {
                  mPush2HeldStepWasEdited = false;
               }
               mPush2ButtonPressTime = gTime;
            }
            else if (index == mPush2HeldStep)
            {
               if (!mPush2HeldStepWasEdited && gTime - mPush2ButtonPressTime < 500)
                  mVels[index] = 0;
               mPush2HeldStep = -1;
            }
         }
         else if (y < sequenceRows + pitchRows)
         {
            if (midiValue > 0)
            {
               int index = x + (pitchRows - 1 - (y - sequenceRows)) * pitchCols;
               if (index < 0 || index >= mNoteRange || x >= pitchCols)
               {
                  //out of range
                  mQueuedPush2Tone = -2;
               }
               else if (mPush2HeldStep != -1)
               {
                  mTones[mPush2HeldStep] = index;
                  mPush2HeldStepWasEdited = true;
               }
               else
               {
                  mQueuedPush2Tone = index;
               }
            }
         }
         else if (y == 7)
         {
            if (midiValue > 0)
            {
               mPush2LengthHeld = true;
               if (mPush2HeldStep != -1)
               {
                  mNoteLengths[mPush2HeldStep] = (x + 1) / 8.0f;
                  mPush2HeldStepWasEdited = true;
               }
               else
               {
                  mQueuedPush2Length = (x + 1) / 8.0f;
               }
            }
            else
            {
               mPush2LengthHeld = false;
            }
         }

         SyncGridToSeq();

         return true;
      }
   }

   if (type == kMidiMessage_PitchBend)
   {
      float val = midiValue / MidiDevice::kPitchBendMax;
      if (mPush2HeldStep != -1)
      {
         mVels[mPush2HeldStep] = int(val * 127);
         mPush2HeldStepWasEdited = true;
      }
      else
      {
         mQueuedPush2Vel = int(val * 127);
      }
      SyncGridToSeq();

      return true;
   }

   return false;
}

void NoteStepSequencer::UpdatePush2Leds(Push2Control* push2)
{
   int sequenceRows, pitchCols, pitchRows;
   GetPush2Layout(sequenceRows, pitchCols, pitchRows);

   int displayStep = std::clamp(mArpIndex, 0, mLength - 1);
   if (mPush2HeldStep != -1)
      displayStep = mPush2HeldStep;

   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor;

         if (y < sequenceRows)
         {
            int index = x + y * 8;
            if (index >= mLength)
               pushColor = 0;
            else if (index == mPush2HeldStep)
               pushColor = 125;
            else if (index == displayStep)
               pushColor = 101;
            else if (mVels[index] > 0)
               pushColor = mNoteLengths[index] == 1 ? 93 : 95;
            else
               pushColor = 92;
         }
         else if (y < sequenceRows + pitchRows)
         {
            int index = x + (pitchRows - 1 - (y - sequenceRows)) * pitchCols;
            int pitch = RowToPitch(index);
            if (x >= pitchCols || index < 0 || index >= mNoteRange)
               pushColor = mQueuedPush2Tone == -2 ? 126 : 0;
            else if (index == mQueuedPush2Tone)
               pushColor = 126;
            else if (index == mTones[displayStep] && ((mVels[displayStep] > 0 && !mAlreadyDidNoteOff) || mPush2HeldStep != -1))
               pushColor = gTime - mLastStepPlayTime[displayStep] < 100 ? 127 : 2;
            else if (TheScale->IsRoot(pitch))
               pushColor = 69;
            else if (TheScale->IsInPentatonic(pitch))
               pushColor = 77;
            else
               pushColor = 78;
         }
         else if (y == 7)
         {
            float displayLength = 0;
            if (mPush2LengthHeld && mPush2HeldStep == -1)
               displayLength = mQueuedPush2Length;
            else if (mVels[displayStep] > 0)
               displayLength = mNoteLengths[displayStep];

            if (displayLength * 8 - 1 >= x)
               pushColor = 83;
            else
               pushColor = 84;
         }
         else
         {
            pushColor = 0;
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }

   std::string touchStripLights = { 0x00, 0x21, 0x1D, 0x01, 0x01, 0x19 };
   int displayVel = mVels[displayStep];
   if (mPush2VelocityHeld && mPush2HeldStep == -1)
      displayVel = mQueuedPush2Vel;
   for (int i = 0; i < 16; ++i)
   {
      int ledLow = ((i * 2) / 32.0f) < displayVel / 127.0f;
      int ledHigh = ((i * 2 + 1) / 32.0f) < displayVel / 127.0f;
      unsigned char c = ledLow + (ledHigh << 3);
      touchStripLights += c;
   }
   push2->GetDevice()->SendSysEx(touchStripLights);
}

void NoteStepSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(NoteStepSequencer);

   ComputeSliders(0);

   if ((mLastNoteLength < 1 || mHasExternalPulseSource) && !mAlreadyDidNoteOff)
   {
      if (NextBufferTime(true) > mLastNoteEndTime)
      {
         PlayNoteOutput(mLastNoteEndTime, mLastPitch, 0);
         if (mShowStepControls && mLastStepIndex < (int)mStepCables.size() && mLastStepIndex != -1)
            SendNoteToCable(mLastStepIndex, mLastNoteEndTime, mLastPitch, 0);
         mAlreadyDidNoteOff = true;
      }
   }
}

void NoteStepSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;

   Step(time, velocity, flags);
}

void NoteStepSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void NoteStepSequencer::Step(double time, float velocity, int pulseFlags)
{
   if (!mEnabled)
      return;

   int direction = 1;
   if (pulseFlags & kPulseFlag_Backward)
      direction = -1;
   if (pulseFlags & kPulseFlag_Repeat)
      direction = 0;

   mArpIndex += direction;
   if (direction > 0 && mArpIndex >= mLength)
      mArpIndex -= (mLength - mLoopResetPoint);
   if (direction < 0 && mArpIndex < mLoopResetPoint)
      mArpIndex += (mLength - mLoopResetPoint);

   if (pulseFlags & kPulseFlag_Reset)
      mArpIndex = 0;
   else if (pulseFlags & kPulseFlag_Random)
      mArpIndex = gRandom() % mLength;

   if (!mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport))
   {
      mArpIndex = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mLength);
   }

   if (pulseFlags & kPulseFlag_Align)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int numMeasures = ceil(float(mLength) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      int step = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % mLength;
      mArpIndex = step;
   }

   int offPitch = -1;
   int offStep = -1;
   if (mLastPitch >= 0 && !mAlreadyDidNoteOff)
   {
      offPitch = mLastPitch;
      offStep = mLastStepIndex;
   }

   if (mQueuedPush2Tone != -1)
   {
      if (mQueuedPush2Tone == -2)
      {
         mVels[mArpIndex] = 0;
      }
      else
      {
         mTones[mArpIndex] = mQueuedPush2Tone;
         mVels[mArpIndex] = mQueuedPush2Vel;
         mNoteLengths[mArpIndex] = mQueuedPush2Length;
      }
      mQueuedPush2Tone = -1;
      mGridSyncQueued = true;
   }

   int current = mTones[mArpIndex];
   if (mVels[mArpIndex] <= 1)
   {
      mLastPitch = -1;
   }
   else
   {
      int outPitch = RowToPitch(current);

      if (mLastPitch == outPitch && !mAlreadyDidNoteOff) //same note, play noteoff first
      {
         PlayNoteOutput(time, mLastPitch, 0, -1);
         if (mShowStepControls && mLastStepIndex < (int)mStepCables.size() && mLastStepIndex != -1)
            SendNoteToCable(mLastStepIndex, time, mLastPitch, 0);
         mAlreadyDidNoteOff = true;
         offPitch = -1;
      }
      if (mVels[mArpIndex] > 1)
      {
         PlayNoteOutput(time, outPitch, mVels[mArpIndex] * velocity, -1);
         if (mShowStepControls && mArpIndex < (int)mStepCables.size())
            SendNoteToCable(mArpIndex, time, outPitch, mVels[mArpIndex] * velocity);
         mLastPitch = outPitch;
         mLastStepIndex = mArpIndex;
         mLastNoteLength = mNoteLengths[mArpIndex];
         mLastNoteEndTime = time + mLastNoteLength * TheTransport->GetDuration(mInterval);
         mLastStepPlayTime[mArpIndex] = time;
         mAlreadyDidNoteOff = false;
      }
   }

   if (offPitch != -1)
   {
      PlayNoteOutput(time, offPitch, 0, -1);
      if (mShowStepControls && offStep < (int)mStepCables.size())
         SendNoteToCable(offStep, time, offPitch, 0);
      if (offPitch == mLastPitch)
      {
         mLastPitch = -1;
      }
   }

   mGrid->SetHighlightCol(time, mArpIndex);
   mVelocityGrid->SetHighlightCol(time, mArpIndex);

   bool isPowerOfTwo = (mLength & (mLength - 1)) == 0;
   int majorColSize = 4;
   bool isAligned = !mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport) || (pulseFlags & kPulseFlag_Align);
   if (isPowerOfTwo && majorColSize < mLength && isAligned)
      mGrid->SetMajorColSize(majorColSize);
   else
      mGrid->SetMajorColSize(-1);

   UpdateLights();
   UpdateGridControllerLights(false);
}

void NoteStepSequencer::SendNoteToCable(int index, double time, int pitch, int velocity)
{
   mStepCables[index]->PlayNoteOutput(time, pitch, velocity);
}

void NoteStepSequencer::UpdateLights()
{
   if (mController)
   {
      for (int i = 0; i < NSS_MAX_STEPS; ++i)
      {
         int button = StepToButton(i);
         int color = 0;
         if (i < mLength)
         {
            if (i == mGrid->GetHighlightCol(NextBufferTime(true)))
            {
               color = LaunchpadInterpreter::LaunchpadColor(0, 3);
            }
            else if (mVels[i] > 1)
            {
               int level = (mVels[i] / 50) + 1;
               color = LaunchpadInterpreter::LaunchpadColor(level, level);
            }
            else
            {
               color = LaunchpadInterpreter::LaunchpadColor(1, 0);
            }
         }
         mController->SendNote(0, button, color);
      }
   }
}

int NoteStepSequencer::ButtonToStep(int button)
{
   if ((button >= 9 && button <= 12) ||
       (button >= 25 && button <= 28))
   {
      return button <= 12 ? (button - 9) : (button - 21);
   }
   return -1;
}

int NoteStepSequencer::StepToButton(int step)
{
   return step < 4 ? (step + 9) : (step + 21);
}

float NoteStepSequencer::ExtraWidth() const
{
   return 10;
}

float NoteStepSequencer::ExtraHeight() const
{
   float height = 103;
   if (mLoopResetPointSlider->IsShowing())
      height += 17;
   if (mShowStepControls)
      height += 17 * 3 + 5;
   return height;
}

void NoteStepSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + ExtraWidth();
   height = mGrid->GetHeight() + ExtraHeight();
}

void NoteStepSequencer::Resize(float w, float h)
{
   mGrid->SetDimensions(MAX(w - ExtraWidth(), 210), MAX(h - ExtraHeight(), 80));
   UpdateVelocityGridPos();
}

void NoteStepSequencer::UpdateVelocityGridPos()
{
   mVelocityGrid->SetDimensions(mGrid->GetWidth(), 45);
   float gridX, gridY;
   mGrid->GetPosition(gridX, gridY, true);
   mVelocityGrid->SetPosition(gridX, gridY + mGrid->GetHeight());
   mLoopResetPointSlider->PositionTo(mVelocityGrid, kAnchor_Below);
   mLoopResetPointSlider->SetDimensions(mVelocityGrid->GetWidth(), 15);
}

void NoteStepSequencer::OnMidiNote(MidiNote& note)
{
   int step = ButtonToStep(note.mPitch);
   if (step != -1 && note.mVelocity > 0)
   {
      if (mSetLength)
      {
         mLength = step + 1;
         UpdateLights();
      }
      else
      {
         mArpIndex = step - 1;
      }
   }
}

void NoteStepSequencer::OnMidiControl(MidiControl& control)
{
   if (control.mControl >= 21 && control.mControl <= 28)
   {
      int step = control.mControl - 21;
      mTones[step] = MIN(control.mValue / 127.0f * mNoteRange, mNoteRange - 1);
      SyncGridToSeq();
   }

   if (control.mControl >= 41 && control.mControl <= 48)
   {
      int step = control.mControl - 41;
      if (control.mValue >= 1)
         mVels[step] = control.mValue;
      SyncGridToSeq();
      UpdateLights();
   }

   if (control.mControl == 115)
   {
      mSetLength = control.mValue > 0;
      if (mController)
         mController->SendCC(0, 116, control.mValue);
   }
}

void NoteStepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mHasExternalPulseSource = true;
      mArpIndex = pitch % mLength;
      Step(time, velocity / 127.0f, kPulseFlag_Repeat);
   }
}

void NoteStepSequencer::ShiftSteps(int amount)
{
   int newTones[NSS_MAX_STEPS];
   int newVels[NSS_MAX_STEPS];
   float newLengths[NSS_MAX_STEPS];
   memcpy(newTones, mTones, NSS_MAX_STEPS * sizeof(int));
   memcpy(newVels, mVels, NSS_MAX_STEPS * sizeof(int));
   memcpy(newLengths, mNoteLengths, NSS_MAX_STEPS * sizeof(float));
   for (int i = 0; i < mLength; ++i)
   {
      int dest = (i + mLength + amount) % mLength;
      newTones[dest] = mTones[i];
      newVels[dest] = mVels[i];
      newLengths[dest] = mNoteLengths[i];
   }
   memcpy(mTones, newTones, NSS_MAX_STEPS * sizeof(int));
   memcpy(mVels, newVels, NSS_MAX_STEPS * sizeof(int));
   memcpy(mNoteLengths, newLengths, NSS_MAX_STEPS * sizeof(float));
   SyncGridToSeq();
}

void NoteStepSequencer::UpdateGridControllerLights(bool force)
{
   if (mGridControlTarget->GetGridController())
   {
      for (int x = 0; x < mGridControlTarget->GetGridController()->NumCols(); ++x)
      {
         for (int y = 0; y < mGridControlTarget->GetGridController()->NumRows(); ++y)
         {
            int column = x + mGridControlOffsetX;
            int row = y - mGridControlOffsetY;

            GridColor color = GridColor::kGridColorOff;
            bool isHighlightCol = (column == mGrid->GetHighlightCol(NextBufferTime(true)));
            if (isHighlightCol)
               color = GridColor::kGridColor2Dim;
            if (column < mLength)
            {
               if (mTones[column] == mGridControlTarget->GetGridController()->NumRows() - 1 - row && mVels[column] > 0)
               {
                  if (isHighlightCol)
                     color = GridColor::kGridColor3Bright;
                  else
                     color = GridColor::kGridColor1Bright;
               }
            }
            mGridControlTarget->GetGridController()->SetLight(x, y, color, force);
         }
      }
   }
}

void NoteStepSequencer::OnControllerPageSelected()
{
   UpdateGridControllerLights(true);
}

void NoteStepSequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int col = x + mGridControlOffsetX;
   int row = y - mGridControlOffsetY;
   if (grid == mGridControlTarget->GetGridController() && col >= 0 && col < mLength && velocity > 0)
   {
      int tone = mGridControlTarget->GetGridController()->NumRows() - 1 - row;
      if (mTones[col] == tone && mVels[col] > 0)
      {
         mVels[col] = 0;
      }
      else
      {
         mTones[col] = tone;
         mVels[col] = velocity * 127;
      }
      SyncGridToSeq();
      UpdateGridControllerLights(false);
   }
}

void NoteStepSequencer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mShiftBackButton)
      ShiftSteps(-1);
   if (button == mShiftForwardButton)
      ShiftSteps(1);
   if (button == mClearButton)
   {
      for (int i = 0; i < mLength; ++i)
         mVels[i] = 0;
      SyncGridToSeq();
   }
   if (button == mRandomizeAllButton)
   {
      RandomizePitches(GetKeyModifiers() & kModifier_Shift);
      RandomizeVelocities();
      RandomizeLengths();
      SyncGridToSeq();
   }
   if (button == mRandomizePitchButton)
   {
      RandomizePitches(GetKeyModifiers() & kModifier_Shift);
      SyncGridToSeq();
   }
   if (button == mRandomizeVelocityButton)
   {
      RandomizeVelocities();
      SyncGridToSeq();
   }
   if (button == mRandomizeLengthButton)
   {
      RandomizeLengths();
      SyncGridToSeq();
   }
}

void NoteStepSequencer::RandomizePitches(bool fifths)
{
   if (fifths)
   {
      for (int i = 0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
         {
            switch (gRandom() % 5)
            {
               case 0:
                  mTones[i] = 0;
                  break;
               case 1:
                  mTones[i] = 4;
                  break;
               case 2:
                  mTones[i] = 7;
                  break;
               case 3:
                  mTones[i] = 11;
                  break;
               case 4:
                  mTones[i] = 14;
                  break;
            }
         }
      }
   }
   else
   {
      //reduce overall randomness: choose from a limited pool of pitches
      std::vector<int> newTones;
      for (int i = 0; i < mRandomizePitchVariety; ++i)
         newTones.push_back(ofClamp(int(ofRandom(0, mNoteRange) + .5f), 0, mNoteRange - 1));

      for (int i = 0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
            mTones[i] = newTones[gRandom() % newTones.size()];
      }
   }
}

void NoteStepSequencer::RandomizeVelocities()
{
   for (int i = 0; i < mLength; ++i)
   {
      if (ofRandom(1) <= mRandomizeVelocityChance)
      {
         int newVelocity = 0;
         if (ofRandom(1) < mRandomizeVelocityDensity)
         {
            switch (gRandom() % 4)
            {
               case 0:
                  newVelocity = 50;
                  break;
               case 1:
                  newVelocity = 80;
                  break;
               case 2:
                  newVelocity = 110;
                  break;
               default:
                  newVelocity = 127;
                  break;
            }
         }

         mVels[i] = newVelocity;
      }
   }
}

void NoteStepSequencer::RandomizeLengths()
{
   for (int i = 0; i < mLength; ++i)
   {
      if (ofRandom(1) <= mRandomizeLengthChance)
      {
         float newLength = ofClamp(ofRandom(2), FLT_EPSILON, 1);
         mNoteLengths[i] = ofLerp(mNoteLengths[i], newLength, mRandomizeLengthRange);
      }
   }
}

void NoteStepSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
   if (list == mNoteModeSelector)
   {
      if (mNoteMode != oldVal)
         mRowOffset = 0;
      SetUpStepControls();
   }
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      if (list == mToneDropdowns[i])
         SyncGridToSeq();
   }
}

void NoteStepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      if (slider == mLengthSliders[i])
         SyncGridToSeq();
   }
}

void NoteStepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLoopResetPointSlider || slider == mLengthSlider)
      mLoopResetPoint = MIN(mLoopResetPoint, mLength - 1);
   if (slider == mLengthSlider)
   {
      mLength = MIN(mLength, NSS_MAX_STEPS);

      if (mLength > oldVal)
      {
         //slice the loop into the nearest power of 2 and loop new steps from there
         int oldLengthPow2 = std::max(1, MathUtils::HighestPow2(oldVal));
         for (int i = oldVal; i < mLength; ++i)
         {
            int loopedFrom = i % oldLengthPow2;
            mTones[i] = mTones[loopedFrom];
            mVels[i] = mVels[loopedFrom];
            mNoteLengths[i] = mNoteLengths[loopedFrom];
         }
      }

      mGrid->SetGrid(mLength, mNoteRange);
      mVelocityGrid->SetGrid(mLength, 1);
      mLoopResetPointSlider->SetExtents(0, mLength);
      SyncGridToSeq();
   }
   if (slider == mOctaveSlider)
      SetUpStepControls();
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      if (slider == mVelocitySliders[i])
         SyncGridToSeq();
   }
   if (slider == mGridControlOffsetXSlider || slider == mGridControlOffsetYSlider)
      UpdateGridControllerLights(false);
}

void NoteStepSequencer::SyncGridToSeq()
{
   mGrid->Clear();
   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      if (mTones[i] < 0)
         continue;

      mGrid->SetVal(i, mTones[i], mVels[i] > 0 ? mNoteLengths[i] : 0, false);
      mVelocityGrid->SetVal(i, 0, mVels[i] / 127.0f, false);
   }
   mGrid->SetGrid(mLength, mNoteRange);
   mVelocityGrid->SetGrid(mLength, 1);
   if (mGridControlTarget->GetGridController())
   {
      int maxXOffset = mLength - mGridControlTarget->GetGridController()->NumCols();
      if (maxXOffset >= 0)
         mGridControlOffsetXSlider->SetExtents(0, maxXOffset);
      int maxYOffset = mNoteRange - mGridControlTarget->GetGridController()->NumRows();
      if (maxYOffset >= 0)
         mGridControlOffsetYSlider->SetExtents(0, maxYOffset);

      mGridControlOffsetX = MAX(MIN(mGridControlOffsetX, maxXOffset), 0);
      mGridControlOffsetY = MAX(MIN(mGridControlOffsetY, maxYOffset), 0);
   }
}

void NoteStepSequencer::OnScaleChanged()
{
   SetUpStepControls();
}

void NoteStepSequencer::SetUpStepControls()
{
   if (TheSynth->IsLoadingModule())
      return;

   for (int i = 0; i < NSS_MAX_STEPS; ++i)
   {
      mToneDropdowns[i]->Clear();
      for (int j = mNoteRange - 1; j >= 0; --j)
         mToneDropdowns[i]->AddLabel(NoteName(RowToPitch(j), false, true), j);
   }
}

void NoteStepSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoteStepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);
   mModuleSaveData.LoadInt("gridrows", moduleInfo, 15, 1, 127, K(isTextField));
   mModuleSaveData.LoadInt("gridsteps", moduleInfo, 8, 1, NSS_MAX_STEPS, K(isTextField));
   mModuleSaveData.LoadBool("stepcontrols", moduleInfo, false);
   mModuleSaveData.LoadInt("steplengthsubdivisions", moduleInfo, 2, 1, 8, K(isTextField));

   SetUpFromSaveData();
}

void NoteStepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetMidiController(mModuleSaveData.GetString("controller"));
   mNoteRange = mModuleSaveData.GetInt("gridrows");
   mShowStepControls = mModuleSaveData.GetBool("stepcontrols");
   mStepLengthSubdivisions = mModuleSaveData.GetInt("steplengthsubdivisions");
   UpdateVelocityGridPos();
   SyncGridToSeq();
   SetUpStepControls();
   mGrid->SetClickValueSubdivisions(mStepLengthSubdivisions);
}

void NoteStepSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mGrid->SaveState(out);
   mVelocityGrid->SaveState(out);
   out << mHasExternalPulseSource;
   float width, height;
   GetModuleDimensions(width, height);
   out << width;
   out << height;
}

void NoteStepSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mGrid->LoadState(in);
   mVelocityGrid->LoadState(in);
   GridUpdated(mGrid, 0, 0, 0, 0);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
   if (rev >= 2)
      in >> mHasExternalPulseSource;
   if (rev >= 3)
   {
      float width, height;
      in >> width;
      in >> height;
      Resize(width, height);
   }
}

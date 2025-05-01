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
//  NoteTable.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/1/21
//
//

#include "NoteTable.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"
#include "MathUtils.h"

NoteTable::NoteTable()
{
   for (size_t i = 0; i < mLastColumnPlayTime.size(); ++i)
      mLastColumnPlayTime[i] = -1;
   for (size_t i = 0; i < mPitchPlayTimes.size(); ++i)
      mPitchPlayTimes[i] = -1;
   for (size_t i = 0; i < mQueuedPitches.size(); ++i)
      mQueuedPitches[i] = false;
}

void NoteTable::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   float width, height;
   UIBLOCK(140);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, kMaxLength);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, 0, 7);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNoteModeSelector, "notemode", (int*)(&mNoteMode), 80);
   UIBLOCK_NEWLINE();
   BUTTON(mRandomizePitchButton, "random pitch");
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mRandomizePitchChanceSlider, "rand pitch chance", &mRandomizePitchChance, 0, 1, 2);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mRandomizePitchRangeSlider, "rand pitch range", &mRandomizePitchRange, 0, 1, 2);
   UIBLOCK_NEWLINE();
   BUTTON(mClearButton, "clear");
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mGridControlTarget, new GridControlTarget(UICONTROL_BASICS("grid")));
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetXSlider, "x offset", &mGridControlOffsetX, 0, 16);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetYSlider, "y offset", &mGridControlOffsetY, 0, 16);
   ENDUIBLOCK(width, height);

   mGrid = new UIGrid(this, "uigrid", 5, height + 18, width - 10, 110, mLength, mNoteRange);

   mNoteModeSelector->AddLabel("scale", kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("pentatonic", kNoteMode_Pentatonic);
   mNoteModeSelector->AddLabel("5ths", kNoteMode_Fifths);

   mGrid->SetFlip(true);
   mGrid->SetListener(this);

   for (int i = 0; i < kMaxLength; ++i)
   {
      mColumnCables[i] = new AdditionalNoteCable();
      mColumnCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mColumnCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
      AddPatchCableSource(mColumnCables[i]->GetPatchCableSource());
   }
}

NoteTable::~NoteTable()
{
}

void NoteTable::Init()
{
   IDrawableModule::Init();
}

void NoteTable::Poll()
{
   UpdateGridControllerLights(false);
}

void NoteTable::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   mGridControlOffsetXSlider->SetShowing((mGridControlTarget->GetGridController() != nullptr && mLength > mGridControlTarget->GetGridController()->NumCols()) || mPush2GridDisplayMode == Push2GridDisplayMode::GridView);
   mGridControlOffsetYSlider->SetShowing((mGridControlTarget->GetGridController() != nullptr && mNoteRange > mGridControlTarget->GetGridController()->NumRows()) || mPush2GridDisplayMode == Push2GridDisplayMode::GridView);

   mLengthSlider->Draw();
   mOctaveSlider->Draw();
   mNoteModeSelector->Draw();
   mRandomizePitchButton->Draw();
   mClearButton->Draw();
   mGridControlTarget->Draw();
   mGridControlOffsetXSlider->Draw();
   mGridControlOffsetYSlider->Draw();
   mRandomizePitchChanceSlider->Draw();
   mRandomizePitchRangeSlider->Draw();

   mGrid->Draw();

   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i = 0; i < mGrid->GetCols(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(i, mGrid->GetRows() - 1) + mGrid->GetPosition(true);
      DrawTextNormal(ofToString(i), pos.x + 1, pos.y - 7);
   }
   for (int i = 0; i < mGrid->GetRows(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(0, i - 1) + mGrid->GetPosition(true);
      float scale = MIN(mGrid->IClickable::GetDimensions().y / mGrid->GetRows() - 2, 18);
      DrawTextNormal(NoteName(RowToPitch(i), false, true) + "(" + ofToString(RowToPitch(i)) + ")", pos.x + 4, pos.y - (scale / 8), scale);
   }
   ofPopStyle();

   if (mGridControlTarget->GetGridController() || mPush2GridDisplayMode == Push2GridDisplayMode::GridView)
   {
      int controllerCols = 8;
      int controllerRows = 8;
      if (mGridControlTarget->GetGridController() != nullptr)
      {
         controllerCols = mGridControlTarget->GetGridController()->NumCols();
         controllerRows = mGridControlTarget->GetGridController()->NumRows();
      }

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
   float boxHeight = float(gridH) / mNoteRange;

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
      if (mLastColumnPlayTime[i] != -1)
      {
         float fade = ofClamp(1 - (gTime - mLastColumnPlayTime[i]) / kPlayHighlightDurationMs, 0, 1);
         ofPushStyle();
         ofFill();
         ofSetColor(ofColor::white, ofLerp(20, 80, fade));
         ofRect(mGrid->GetPosition(true).x + mGrid->GetWidth() / mLength * i, mGrid->GetPosition(true).y, mGrid->GetWidth() / mLength, mGrid->GetHeight());
         ofNoFill();
         ofSetLineWidth(3 * fade);
         for (int row = 0; row < mGrid->GetRows(); ++row)
         {
            if (mGrid->GetVal(i, row) > 0)
            {
               ofVec2f pos = mGrid->GetCellPosition(i, row) + mGrid->GetPosition(true);
               float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
               float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();
               ofSetColor(ofColor::white, fade * 255);
               ofRect(pos.x, pos.y, xsize, ysize);
            }
         }
         ofPopStyle();
      }
   }

   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   for (int i = 0; i < kMaxLength; ++i)
   {
      if (i < mLength && mShowColumnCables)
      {
         ofVec2f pos = mGrid->GetCellPosition(i, 0) + mGrid->GetPosition(true);
         pos.x += mGrid->GetWidth() / float(mLength) * .5f;
         pos.y = moduleHeight - 7;
         mColumnCables[i]->GetPatchCableSource()->SetManualPosition(pos.x, pos.y);
         mColumnCables[i]->GetPatchCableSource()->SetEnabled(true);
      }
      else
      {
         mColumnCables[i]->GetPatchCableSource()->SetEnabled(false);
      }
   }

   ofPopStyle();
}

void NoteTable::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mGrid->TestClick(x, y, right);
}

void NoteTable::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool NoteTable::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mGrid->NotifyMouseMoved(x, y);
   return false;
}

void NoteTable::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteTable::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
}

int NoteTable::RowToPitch(int row)
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

int NoteTable::PitchToRow(int pitch)
{
   for (int i = 0; i < mGrid->GetRows(); ++i)
   {
      if (pitch == RowToPitch(i))
         return i;
   }
   return -1;
}

float NoteTable::ExtraWidth() const
{
   return 10;
}

float NoteTable::ExtraHeight() const
{
   float height = 77;
   if (mShowColumnCables)
      height += 22;
   return height;
}

void NoteTable::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + ExtraWidth();
   height = mGrid->GetHeight() + ExtraHeight();
}

void NoteTable::Resize(float w, float h)
{
   mGrid->SetDimensions(MAX(w - ExtraWidth(), 210), MAX(h - ExtraHeight(), 80));
}

void NoteTable::PlayNote(NoteMessage note)
{
   if ((mEnabled || note.velocity == 0) && note.pitch < kMaxLength)
      PlayColumn(note);
}

void NoteTable::PlayColumn(NoteMessage note)
{
   int column = note.pitch;
   if (note.velocity == 0)
   {
      mLastColumnPlayTime[column] = -1;
      for (int i = 0; i < mLastColumnNoteOnPitches.size(); ++i)
      {
         if (mLastColumnNoteOnPitches[column][i])
         {
            PlayNoteOutput(NoteMessage(note.time, i, 0, note.voiceIdx, note.modulation));
            mColumnCables[column]->PlayNoteOutput(NoteMessage(note.time, i, 0, note.voiceIdx, note.modulation));
            mLastColumnNoteOnPitches[column][i] = false;
         }
      }
   }
   else
   {
      mLastColumnPlayTime[column] = note.time;
      for (int row = 0; row < mGrid->GetRows(); ++row)
      {
         int outputPitch = RowToPitch(row);

         // don't play notes > 127, and also to avoid bufferoverflow for mQueuedPitches and mPitchPlayTimes below
         if (outputPitch >= mPitchPlayTimes.size() || outputPitch >= mQueuedPitches.size())
            continue;

         if (mQueuedPitches[outputPitch])
         {
            mGrid->SetVal(column, row, 1);
            mQueuedPitches[outputPitch] = false;
         }

         if (mGrid->GetVal(column, row) == 0)
            continue;

         PlayNoteOutput(NoteMessage(note.time, outputPitch, note.velocity, note.voiceIdx, note.modulation));
         mColumnCables[column]->PlayNoteOutput(NoteMessage(note.time, outputPitch, note.velocity, note.voiceIdx, note.modulation));
         mLastColumnNoteOnPitches[column][outputPitch] = true;
         mPitchPlayTimes[outputPitch] = note.time;
      }
   }
}

void NoteTable::UpdateGridControllerLights(bool force)
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
            if (column < mLength)
            {
               if (mGrid->GetVal(column, row) > 0)
               {
                  if (mLastColumnPlayTime[column] + 80 > gTime)
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

void NoteTable::OnControllerPageSelected()
{
   UpdateGridControllerLights(true);
}

void NoteTable::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int col = x + mGridControlOffsetX;
   int row = y - mGridControlOffsetY;
   if (grid == mGridControlTarget->GetGridController() && col >= 0 && col < mLength && velocity > 0)
   {
      mGrid->SetVal(col, row, mGrid->GetVal(col, row) > 0 ? 0 : 1);
      UpdateGridControllerLights(false);
   }
}

void NoteTable::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizePitchButton)
      RandomizePitches(GetKeyModifiers() & kModifier_Shift);
   if (button == mClearButton)
      mGrid->Clear();
}

void NoteTable::RandomizePitches(bool fifths)
{
   if (fifths)
   {
      for (int i = 0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
         {
            switch (gRandom() % 5)
            {
               default:
                  SetColumnRow(i, 0);
                  break;
               case 1:
                  SetColumnRow(i, 4);
                  break;
               case 2:
                  SetColumnRow(i, 7);
                  break;
               case 3:
                  SetColumnRow(i, 11);
                  break;
               case 4:
                  SetColumnRow(i, 14);
                  break;
            }
         }
      }
   }
   else
   {
      for (int i = 0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
         {
            int row = ofRandom(0, mNoteRange);
            for (int j = 0; j < mNoteRange; ++j)
            {
               if (mGrid->GetVal(i, j) > 0)
                  row = j;
            }
            float minValue = MAX(0, row - mNoteRange * mRandomizePitchRange);
            float maxValue = MIN(mNoteRange, row + mNoteRange * mRandomizePitchRange);
            if (minValue != maxValue)
               SetColumnRow(i, ofClamp(int(ofRandom(minValue, maxValue) + .5f), 0, mNoteRange - 1));
         }
      }
   }
}

void NoteTable::SetColumnRow(int column, int row)
{
   for (int i = 0; i < mNoteRange; ++i)
      mGrid->SetVal(column, i, i == row ? 1 : 0, false);
}

void NoteTable::GetPush2Layout(int& sequenceRows, int& pitchCols, int& pitchRows)
{
   sequenceRows = (mLength - 1) / 8 + 1;
   if (mNoteMode == kNoteMode_Scale && TheScale->NumTonesInScale() == 7)
      pitchCols = 7;
   else
      pitchCols = 8;
   pitchRows = (mNoteRange - 1) / pitchCols + 1;
}

bool NoteTable::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (mPush2GridDisplayMode == Push2GridDisplayMode::PerStep)
   {
      int sequenceRows, pitchCols, pitchRows;
      GetPush2Layout(sequenceRows, pitchCols, pitchRows);

      if (type == kMidiMessage_Note)
      {
         if (controlIndex >= 36 && controlIndex <= 99)
         {
            int gridIndex = controlIndex - 36;
            int x = gridIndex % 8;
            int y = 7 - gridIndex / 8;

            if (y < sequenceRows)
            {
               int index = x + y * 8;
               if (midiValue > 0)
                  mPush2HeldStep = index;
               else if (index == mPush2HeldStep)
                  mPush2HeldStep = -1;
            }
            else if (y < sequenceRows + pitchRows)
            {
               if (midiValue > 0)
               {
                  int index = x + (pitchRows - 1 - (y - sequenceRows)) * pitchCols;
                  if (index < 0 || index >= mNoteRange)
                  {
                     //out of range, do nothing
                  }
                  else if (mPush2HeldStep != -1)
                  {
                     mGrid->SetVal(mPush2HeldStep, index, mGrid->GetVal(mPush2HeldStep, index) > 0 ? 0 : 1);
                  }
                  else
                  {
                     //I'm not liking this "queued pitch" behavior, let's disable it for now
                     //mQueuedPitches[RowToPitch(index)] = true;
                  }
               }
            }

            return true;
         }
      }
   }
   else if (mPush2GridDisplayMode == Push2GridDisplayMode::GridView)
   {
      if (type == kMidiMessage_Note)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = gridIndex / 8;
         int col = x + mGridControlOffsetX;
         int row = y + mGridControlOffsetY;
         if (gridIndex >= 0 && gridIndex < 64 &&
             col >= 0 && col < mLength &&
             row >= 0 && row < mNoteRange)
         {
            if (midiValue > 0)
            {
               mPush2HeldStep = col;
               mGrid->SetVal(mPush2HeldStep, row, mGrid->GetVal(mPush2HeldStep, row) > 0 ? 0 : 1);
            }
            else
            {
               mPush2HeldStep = -1;
            }
         }
         return true;
      }
   }

   if (type == kMidiMessage_Control)
   {
      if (controlIndex == push2->GetGridControllerOption1Control())
      {
         if (midiValue > 0)
         {
            if (mPush2GridDisplayMode == Push2GridDisplayMode::PerStep)
               mPush2GridDisplayMode = Push2GridDisplayMode::GridView;
            else
               mPush2GridDisplayMode = Push2GridDisplayMode::PerStep;
         }
         return true;
      }
   }

   return false;
}

void NoteTable::UpdatePush2Leds(Push2Control* push2)
{
   int sequenceRows, pitchCols, pitchRows;
   GetPush2Layout(sequenceRows, pitchCols, pitchRows);

   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor = 0;

         if (mPush2GridDisplayMode == Push2GridDisplayMode::PerStep)
         {
            if (y < sequenceRows)
            {
               int index = x + y * 8;
               if (index >= mLength)
                  pushColor = 0;
               else if (index == mPush2HeldStep)
                  pushColor = 125;
               else if (mLastColumnPlayTime[index] != -1)
                  pushColor = 101;
               else
                  pushColor = 93;
            }
            else if (y < sequenceRows + pitchRows)
            {
               int index = x + (pitchRows - 1 - (y - sequenceRows)) * pitchCols;
               int pitch = RowToPitch(index);
               if (x >= pitchCols || index >= mNoteRange)
                  pushColor = 0;
               else if (mPush2HeldStep != -1 && mGrid->GetVal(mPush2HeldStep, index) > 0)
                  pushColor = 127;
               else if (mPush2HeldStep == -1 && mNoteOutput.GetNotes()[pitch])
                  pushColor = gTime - mPitchPlayTimes[pitch] < 100 ? 127 : 2;
               else if (mQueuedPitches[pitch])
                  pushColor = 126;
               else if (TheScale->IsRoot(pitch))
                  pushColor = 69;
               else if (TheScale->IsInPentatonic(pitch))
                  pushColor = 77;
               else
                  pushColor = 78;
            }
         }
         else if (mPush2GridDisplayMode == Push2GridDisplayMode::GridView)
         {
            int column = x + mGridControlOffsetX;
            int row = (7 - y) + mGridControlOffsetY;

            if (column >= 0 && column < mLength && row >= 0 && row < mNoteRange)
            {
               bool isHighlightCol = mLastColumnPlayTime[column] != -1;
               int pitch = RowToPitch(row);
               if (isHighlightCol && mPush2HeldStep == -1 && mNoteOutput.GetNotes()[pitch])
                  pushColor = gTime - mPitchPlayTimes[pitch] < 100 ? 127 : 2;
               else if (mGrid->GetVal(column, row) > 0)
                  pushColor = 125;
               else if (mQueuedPitches[pitch])
                  pushColor = 126;
               else if (isHighlightCol)
                  pushColor = 83;
               else if (TheScale->IsRoot(pitch))
                  pushColor = 69;
               else if (TheScale->IsInPentatonic(pitch))
                  pushColor = 77;
               else
                  pushColor = 78;

               /*bool isHighlightCol = mLastColumnPlayTime[column] != -1;
               int pitch = RowToPitch(row);
               if (TheScale->IsRoot(pitch))
                  pushColor = 69;
               else if (TheScale->IsInPentatonic(pitch))
                  pushColor = 77;
               else
                  pushColor = 78;
               if (isHighlightCol)
                  pushColor = 83;
               if (mTones[column] == 8 - 1 - row && mVels[column] > 0)
               {
                  if (column == mPush2HeldStep)
                     pushColor = 127;
                  else if (isHighlightCol)
                     pushColor = 126;
                  else
                     pushColor = mNoteLengths[column] == 1 ? 125 : 95;
               }*/
            }
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }

   push2->SetLed(kMidiMessage_Control, push2->GetGridControllerOption1Control(), 127);
}

void NoteTable::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mNoteModeSelector)
   {
      if (mNoteMode != oldVal)
         mRowOffset = 0;
   }
}

void NoteTable::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLengthSlider)
   {
      mLength = MIN(mLength, kMaxLength);

      if (mLength > oldVal)
      {
         //slice the loop into the nearest power of 2 and loop new steps from there
         int oldLengthPow2 = std::max(1, MathUtils::HighestPow2(oldVal));
         for (int i = oldVal; i < mLength; ++i)
         {
            int loopedFrom = i % oldLengthPow2;
            for (int row = 0; row < mGrid->GetRows(); ++row)
               mGrid->SetVal(i, row, mGrid->GetVal(loopedFrom, row));
         }
      }

      mGrid->SetGrid(mLength, mNoteRange);

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
   if (slider == mGridControlOffsetXSlider || slider == mGridControlOffsetYSlider)
      UpdateGridControllerLights(false);
}

void NoteTable::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoteTable::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadInt("gridrows", moduleInfo, 15, 1, 127, K(isTextField));
   mModuleSaveData.LoadBool("columncables", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteTable::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mNoteRange = mModuleSaveData.GetInt("gridrows");
   mShowColumnCables = mModuleSaveData.GetBool("columncables");

   mGrid->SetGrid(mLength, mNoteRange);
}

void NoteTable::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mGrid->SaveState(out);
   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
}

void NoteTable::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mGrid->LoadState(in);
   GridUpdated(mGrid, 0, 0, 0, 0);

   if (rev >= 3)
   {
      float width, height;
      in >> width;
      in >> height;
      mGrid->SetDimensions(width, height);
   }
}

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
//  LaunchpadKeyboard.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#include "LaunchpadKeyboard.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Chorder.h"
#include "MidiController.h"
#include "FillSaveDropdown.h"
#include "AbletonDeviceShared.h"

#define INVALID_PITCH -999
#define CHORD_BUTTON_OFFSET -100
#define CHORD_ENABLE_BUTTON -998
#define CHORD_LATCH_BUTTON -997
#define KEY_LATCH_BUTTON -996

LaunchpadKeyboard::LaunchpadKeyboard()
{
   for (int i = 0; i < 128; ++i)
      mCurrentNotes[i] = 0;

   TheScale->AddListener(this);


   mHeldChordTones.push_back(0);

   std::vector<int> chord;
   //triad
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //7
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //6
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(9);
   mChords.push_back(chord);
   chord.clear();

   //inv
   chord.push_back(-5);
   chord.push_back(0);
   chord.push_back(4);
   mChords.push_back(chord);
   chord.clear();

   //sus4
   chord.push_back(0);
   chord.push_back(5);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //sus2
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //9
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //11
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(4);
   chord.push_back(5);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //africa
   chord.push_back(-12);
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(12);
   chord.push_back(16);
   chord.push_back(28);
   mChords.push_back(chord);
   chord.clear();
}

void LaunchpadKeyboard::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_8n, OffsetInfo(0, true), false);
}

void LaunchpadKeyboard::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLayoutDropdown = new DropdownList(this, "layout", 6, 22, (int*)(&mLayout));
   mOctaveSlider = new IntSlider(this, "octave", 6, 40, 100, 15, &mOctave, 0, 8);
   mLatchCheckbox = new Checkbox(this, "latch", 6, 59, &mLatch);
   mArrangementModeDropdown = new DropdownList(this, "arrangement", 6, 4, ((int*)(&mArrangementMode)));
   mLatchChordsCheckbox = new Checkbox(this, "ch.latch", 55, 59, &mLatchChords);
   mPreserveChordRootCheckbox = new Checkbox(this, "p.root", 70, 4, &mPreserveChordRoot);
   mGridControlTarget = new GridControlTarget(this, "grid", 90, 22);

   mLayoutDropdown->AddLabel("chromatic", (int)LaunchpadLayout::kChromatic);
   mLayoutDropdown->AddLabel("diatonic", (int)LaunchpadLayout::kDiatonic);
   mLayoutDropdown->AddLabel("major thirds", (int)LaunchpadLayout::kMajorThirds);
   mLayoutDropdown->AddLabel("chord indiv", (int)LaunchpadLayout::kChordIndividual);
   mLayoutDropdown->AddLabel("chord", (int)LaunchpadLayout::kChord);
   mLayoutDropdown->AddLabel("guitar", (int)LaunchpadLayout::kGuitar);
   mLayoutDropdown->AddLabel("septatonic", (int)LaunchpadLayout::kSeptatonic);
   mLayoutDropdown->AddLabel("drum", (int)LaunchpadLayout::kDrum);
   mLayoutDropdown->AddLabel("all pads", (int)LaunchpadLayout::kAllPads);
   mLayoutDropdown->AddLabel("piano", (int)LaunchpadLayout::kPiano);
   mLayoutDropdown->AddLabel("scale rows", (int)LaunchpadLayout::kScaleRows);
   mLayoutDropdown->AddLabel("pentatonic", (int)LaunchpadLayout::kPentatonic);

   mArrangementModeDropdown->AddLabel("full", (int)ArrangementMode::kFull);
   mArrangementModeDropdown->AddLabel("five", (int)ArrangementMode::kFive);
   mArrangementModeDropdown->AddLabel("six", (int)ArrangementMode::kSix);
}

LaunchpadKeyboard::~LaunchpadKeyboard()
{
   TheScale->RemoveListener(this);
   TheTransport->RemoveListener(this);
}

void LaunchpadKeyboard::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   bool bOn = velocity > 0;
   int pitch = GridToPitch(x, y);
   double time = NextBufferTime(false);

   if (pitch == INVALID_PITCH)
   {
      ReleaseNoteFor(x, y);
      return;
   }
   if (pitch == CHORD_ENABLE_BUTTON)
   {
      if (bOn && mChorder)
         mChorder->SetEnabled(!mChorder->IsEnabled());
      return;
   }
   if (pitch == CHORD_LATCH_BUTTON)
   {
      if (bOn)
      {
         mLatchChords = !mLatchChords;
         UpdateLights();
      }
      return;
   }
   if (pitch == KEY_LATCH_BUTTON)
   {
      if (bOn)
      {
         mLatch = !mLatch;
         if (!mLatch)
            mNoteOutput.Flush(NextBufferTime(false));
         UpdateLights();
      }
      return;
   }
   if (pitch < 0)
   {
      HandleChordButton(pitch, bOn);

      UpdateLights();

      return;
   }
   if (!mLatch && mLayout != LaunchpadLayout::kChord)
   {
      //handled below
   }
   else if (mLatch)
   {
      if (bOn) //only presses matter in latch, not releases
      {
         int currentPitch = -1;
         for (int i = 0; i < 128; ++i) //we should only have one at a time in latch mode
         {
            if (mCurrentNotes[i] > 0)
               currentPitch = i;
            mCurrentNotes[i] = 0;
         }
         mNoteOutput.Flush(NextBufferTime(false));

         if (currentPitch == pitch)
         {
            bOn = false; //pressed the note again, this is a note-off
         }
         else
         {
            PressedNoteFor(x, y, (int)127 * velocity);
            bOn = true;
         }
      }
      else
      {
         return;
      }
   }

   if (mLayout == LaunchpadLayout::kChord)
   {
      if (bOn)
      {
         for (int i = 0; i < 128; ++i)
            mCurrentNotes[i] = 0;
         PressedNoteFor(x, y, (int)127 * velocity);
         mNoteOutput.Flush(time);
         for (int i = 0; i < mChords[x].size(); ++i)
            PlayKeyboardNote(time, TheScale->MakeDiatonic(pitch + mChords[x][i]), 127 * velocity);
      }
      else
      {
         int currentPitch = -1;
         for (int i = 0; i < 128; ++i)
         {
            if (mCurrentNotes[i] > 0)
               currentPitch = i;
         }

         if (GridToPitch(x, y) == currentPitch)
         {
            for (int i = 0; i < 128; ++i)
               mCurrentNotes[i] = 0;
            mNoteOutput.Flush(time);
         }
      }
   }
   else
   {
      if (bOn)
      {
         PlayKeyboardNote(time, pitch, 127 * velocity);
         PressedNoteFor(x, y, (int)127 * velocity);
      }
      else
      {
         ReleaseNoteFor(x, y);
      }
   }

   if (mDisplayer == nullptr) //we don't have a displayer, handle it ourselves
      UpdateLights();
}

void LaunchpadKeyboard::PressedNoteFor(int x, int y, int velocity)
{
   int pitch = GridToPitch(x, y);
   if (pitch >= 0 && pitch < 128)
      mCurrentNotes[pitch] = velocity;
}

void LaunchpadKeyboard::ReleaseNoteFor(int x, int y)
{
   int pitch = GridToPitch(x, y);
   if (pitch >= 0 && pitch < 128)
   {
      double time = NextBufferTime(false);
      PlayKeyboardNote(time, pitch, 0);
      mCurrentNotes[pitch] = 0;
   }
}

void LaunchpadKeyboard::PlayKeyboardNote(double time, int pitch, int velocity)
{
   if (mEnabled || velocity == 0)
   {
      if (velocity == 0)
         time += .001f; //TODO(Ryan) gross hack. need to handle the case better of receiving a note-on followed by a note-off for one pitch at the exact same time. right now it causes stuck notes.
      PlayNoteOutput(NoteMessage(time, pitch, velocity, 0, ModulationParameters(mModulation.GetPitchBend(0), mModulation.GetModWheel(0), mModulation.GetPressure(0), 0)));
   }

   if (mDrawDebug)
      AddDebugLine("PlayNote(" + ofToString(time / 1000) + ", " + ofToString(pitch) + ", " + ofToString(velocity) + ")", 40);
}

void LaunchpadKeyboard::HandleChordButton(int pitch, bool bOn)
{
   int chordTone = pitch - CHORD_BUTTON_OFFSET;

   if (mPreserveChordRoot && chordTone == 0) //root always pressed
      return;

   if (!mLatchChords)
   {
      if (bOn)
      {
         mHeldChordTones.push_back(chordTone);
         if (mChorder)
            mChorder->AddTone(chordTone);
      }
      else
      {
         mHeldChordTones.remove(chordTone);
         if (mChorder)
            mChorder->RemoveTone(chordTone);
      }
   }
   else if (bOn) //latch only pays attention to presses
   {
      if (!ListContains(chordTone, mHeldChordTones))
      {
         mHeldChordTones.push_back(chordTone);
         if (mChorder)
            mChorder->AddTone(chordTone);
      }
      else
      {
         mHeldChordTones.remove(chordTone);
         if (mChorder)
            mChorder->RemoveTone(chordTone);
      }
   }
}

bool LaunchpadKeyboard::IsChordButtonPressed(int pitch)
{
   int chordTone = pitch - CHORD_BUTTON_OFFSET;

   if (mPreserveChordRoot && chordTone == 0) //root always pressed
      return true;

   return ListContains(chordTone, mHeldChordTones);
}

void LaunchpadKeyboard::OnTimeEvent(double time)
{
}

bool LaunchpadKeyboard::OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue)
{
   if (controlIndex >= AbletonDevice::kChannelPressureIndex && controlIndex < AbletonDevice::kChannelPressureIndex + AbletonDevice::kNumChannelPressureIndices)
   {
      int channel = 0; //controlIndex - AbletonDevice::kChannelPressureIndex;
      mModulation.GetPressure(channel)->SetValue(midiValue / 127.0f);
      mNoteOutput.SendPressure(channel, midiValue / 127.0f);
      return true;
   }

   int rangeStart = abletonGrid->GetGridStartIndex();
   int rangeEnd = abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads();
   mCols = abletonGrid->GetGridNumCols(); //TODO(Ryan) need proper way to update grid size based upon context of relevant grid
   mRows = abletonGrid->GetGridNumRows(); //TODO(Ryan) need proper way to update grid size based upon context of relevant grid

   if (controlIndex >= rangeStart && controlIndex < rangeEnd)
   {
      int gridIndex = controlIndex - rangeStart;
      int gridX = gridIndex % abletonGrid->GetGridNumCols();
      int gridY = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();
      OnGridButton(gridX, gridY, midiValue / 127, nullptr);
      return true;
   }

   if (controlIndex == AbletonDevice::kOctaveUpButton)
   {
      if (midiValue > 0)
      {
         AdjustOctave(1);
         abletonGrid->DisplayScreenMessage("Octave: " + ofToString(mOctave));
         return true;
      }
   }
   else if (controlIndex == AbletonDevice::kOctaveDownButton)
   {
      if (midiValue > 0)
      {
         AdjustOctave(-1);
         abletonGrid->DisplayScreenMessage("Octave: " + ofToString(mOctave));
         return true;
      }
   }

   return false;
}

void LaunchpadKeyboard::UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid)
{
   int offset = abletonGrid->GetGridStartIndex();
   mCols = abletonGrid->GetGridNumCols(); //TODO(Ryan) need proper way to update grid size based upon context of relevant grid
   mRows = abletonGrid->GetGridNumRows(); //TODO(Ryan) need proper way to update grid size based upon context of relevant grid

   for (int x = 0; x < abletonGrid->GetGridNumCols(); ++x)
   {
      for (int y = 0; y < abletonGrid->GetGridNumRows(); ++y)
      {
         int pitch = GridToPitch(x, y);
         bool isPreviewNote = pitch >= 0 && pitch < 128 && mPreviewNotes[pitch];
         bool isCurrentNote = pitch >= 0 && pitch < 128 && mCurrentNotes[pitch] > 0;

         GridColor color = GetGridSquareColor(x, y);
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

         if (isPreviewNote)
            pushColor = AbletonDevice::kColorWhite;

         abletonGrid->SetLed(x + (abletonGrid->GetGridNumRows() - 1 - y) * abletonGrid->GetGridNumCols() + offset, pushColor);
         //abletonGrid->SetLed(x + (7-y)*8 + 36, x + y*8 + 64);
      }
   }
}

void LaunchpadKeyboard::DisplayNote(int pitch, int velocity)
{
   if (pitch >= 0 && pitch < (int)mCurrentNotes.size())
      mCurrentNotes[pitch] = velocity;

   UpdateLights();
}

void LaunchpadKeyboard::SetPreviewNotes(const std::function<bool(int)>& IsPreviewNote)
{
   for (int i = 0; i < (int)mPreviewNotes.size(); ++i)
      mPreviewNotes[i] = IsPreviewNote(i);

   UpdateLights();
}

void LaunchpadKeyboard::AdjustOctave(int amount)
{
   mOctave = std::clamp(mOctave + amount, mOctaveSlider->GetMin(), mOctaveSlider->GetMax());
}

void LaunchpadKeyboard::DrawModule()
{
   if (mChorder)
      DrawConnection(mChorder);
   if (Minimized() || IsVisible() == false)
      return;
   mLayoutDropdown->Draw();
   mOctaveSlider->Draw();
   mLatchCheckbox->Draw();
   mLatchChordsCheckbox->Draw();
   mArrangementModeDropdown->Draw();
   mPreserveChordRootCheckbox->Draw();
   mGridControlTarget->Draw();
}

void LaunchpadKeyboard::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      DrawTextNormal(mDebugDisplayText, 0, 90);

      for (int i = 0; i < 128; ++i)
         DrawTextNormal(ofToString(i) + " " + ofToString(mCurrentNotes[i]), 180 + (i / 24) * 20, (i % 24) * 9, 6);
   }
}

int LaunchpadKeyboard::GridToPitch(int x, int y)
{
   y = mRows - 1 - y;
   if (mArrangementMode == ArrangementMode::kSix)
   {
      if (x < 2)
      {
         return GridToPitchChordSection(x, y);
      }
      else
      {
         x -= 2;
      }
      return TheScale->ScaleRoot() + x + 6 * y + TheScale->GetPitchesPerOctave() * mOctave;
   }
   if (mLayout == LaunchpadLayout::kChromatic)
   {
      if (mArrangementMode == ArrangementMode::kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x, y);
         }
         else
         {
            x -= 3;
         }
      }
      return mRootNote + x + 5 * y + TheScale->GetPitchesPerOctave() * mOctave;
   }
   if (mLayout == LaunchpadLayout::kMajorThirds)
   {
      if (mArrangementMode == ArrangementMode::kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x, y);
         }
         else
         {
            x -= 3;
         }
      }
      return mRootNote + x + 4 * y + TheScale->GetPitchesPerOctave() * mOctave;
   }
   if (mLayout == LaunchpadLayout::kGuitar)
   {
      return mRootNote + x + 5 * y + TheScale->GetPitchesPerOctave() * mOctave + (y >= 4 ? -1 : 0);
   }
   else if (mLayout == LaunchpadLayout::kDiatonic)
   {
      if (mArrangementMode == ArrangementMode::kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x, y);
         }
         else if (x == 3 || x == 7)
         {
            return INVALID_PITCH;
         }
         else
         {
            x -= 4;
         }
      }
      return TheScale->GetPitchFromTone(x + 3 * y) + TheScale->GetPitchesPerOctave() * (mRootNote / TheScale->GetPitchesPerOctave()) + TheScale->GetPitchesPerOctave() * mOctave;
   }
   else if (mLayout == LaunchpadLayout::kChordIndividual)
   {
      int note = x % mChords[mCurrentChord].size();
      int oct = x / mChords[mCurrentChord].size();
      return TheScale->MakeDiatonic(TheScale->GetPitchFromTone(y) + mChords[mCurrentChord][note]) + TheScale->GetPitchesPerOctave() * (mOctave + oct);
   }
   else if (mLayout == LaunchpadLayout::kChord)
   {
      if (x < mChords.size())
         return TheScale->GetPitchFromTone(y) + TheScale->GetPitchesPerOctave() * mOctave;
      else
         return INVALID_PITCH;
   }
   else if (mLayout == LaunchpadLayout::kSeptatonic)
   {
      if (mArrangementMode == ArrangementMode::kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x, y);
         }
         else if (x == 3)
         {
            if (TheScale->NumTonesInScale() == 7) //septatonic scales only
            {
               if (y % 2 == 0) // 7 or maj7
               {
                  int nonDiatonic = TheScale->GetPitchFromTone(0) - 1 + TheScale->GetPitchesPerOctave() * (mRootNote / TheScale->GetPitchesPerOctave()) + TheScale->GetPitchesPerOctave() * (mOctave + y / 2);
                  if (TheScale->IsInScale(nonDiatonic))
                     --nonDiatonic;
                  return nonDiatonic;
               }
               if (y % 2 == 1) // 4 or sharp 4
               {
                  int nonDiatonic = TheScale->GetPitchFromTone(4) - 1 + TheScale->GetPitchesPerOctave() * (mRootNote / TheScale->GetPitchesPerOctave()) + TheScale->GetPitchesPerOctave() * (mOctave + y / 2);
                  if (TheScale->IsInScale(nonDiatonic))
                     --nonDiatonic;
                  return nonDiatonic;
               }
            }
            return INVALID_PITCH;
         }
         else
         {
            x -= 4;
         }
      }

      int numPitchesInScale = TheScale->NumTonesInScale();
      if (numPitchesInScale > 8)
         return INVALID_PITCH;

      int pos = x + 4 * y;
      int set = pos / 8;
      int tone = pos - set * (8 - numPitchesInScale); // + TheScale->GetScaleDegree(); add this for chord following
      if (pos % 8 >= numPitchesInScale)
         return INVALID_PITCH;

      return TheScale->GetPitchFromTone(tone) + TheScale->GetPitchesPerOctave() * (mRootNote / TheScale->GetPitchesPerOctave()) + TheScale->GetPitchesPerOctave() * mOctave;
   }
   else if (mLayout == LaunchpadLayout::kDrum)
   {
      if (x < 4 && y < 4)
         return x + y * 4 + mOctave * 12;
      else
         return INVALID_PITCH;
   }
   else if (mLayout == LaunchpadLayout::kAllPads)
   {
      return x + y * 8;
   }
   else if (mLayout == LaunchpadLayout::kPiano)
   {
      int octave = (x / 7) + y / 2 * std::max(mCols / 7, 1);
      if (y % 2 == 1) //black keys
      {
         const int kBlackKeys[] = { INVALID_PITCH, 1, 3, INVALID_PITCH, 6, 8, 10 };
         if ((x % 7) == 0 || (x % 7) == 3)
            return INVALID_PITCH;
         return kBlackKeys[x % 7] + (mOctave + octave) * 12;
      }
      else //white keys
      {
         const int kWhiteKeys[] = { 0, 2, 4, 5, 7, 9, 11 };
         return kWhiteKeys[x % 7] + (mOctave + octave) * 12;
      }
   }
   else if (mLayout == LaunchpadLayout::kScaleRows)
   {
      return TheScale->GetPitchFromTone(x) + (y + mOctave) * TheScale->GetPitchesPerOctave();
   }
   else if (mLayout == LaunchpadLayout::kPentatonic)
   {
      bool isMinor = TheScale->IsInScale(TheScale->ScaleRoot() + 3);
      const int kMinorPentatonic[] = { 0, 3, 5, 7, 10 };
      const int kMajorPentatonic[] = { 0, 2, 4, 7, 9 };
      int number = x + y * mCols;
      int index = number % 5;
      int octave = number / 5;
      return TheScale->ScaleRoot() + (isMinor ? kMinorPentatonic[index] : kMajorPentatonic[index]) + (mOctave + octave) * TheScale->GetPitchesPerOctave();
   }

   return 0;
}

int LaunchpadKeyboard::GridToPitchChordSection(int x, int y)
{
   int numPitchesInScale = TheScale->NumTonesInScale();

   if (y < 7 && y < numPitchesInScale)
   {
      return CHORD_BUTTON_OFFSET + (numPitchesInScale * (x - 1)) + y;
   }
   else if (y == 7)
   {
      if (x == 0)
         return CHORD_ENABLE_BUTTON;
      if (x == 1)
         return CHORD_LATCH_BUTTON;
      if (x == 2)
         return KEY_LATCH_BUTTON;
   }

   return INVALID_PITCH;
}

int LaunchpadKeyboard::TransposePitchInScale(int pitch, int amount, bool octaveMultiplier)
{
   switch (mLayout)
   {
      case LaunchpadLayout::kChromatic:
      case LaunchpadLayout::kMajorThirds:
      case LaunchpadLayout::kGuitar:
      case LaunchpadLayout::kAllPads:
      case LaunchpadLayout::kPiano:
         return pitch + amount * (octaveMultiplier ? TheScale->GetPitchesPerOctave() : 1);
      case LaunchpadLayout::kDiatonic:
      case LaunchpadLayout::kChordIndividual:
      case LaunchpadLayout::kChord:
      case LaunchpadLayout::kSeptatonic:
      case LaunchpadLayout::kScaleRows:
         return TheScale->GetPitchFromTone(TheScale->GetToneFromPitch(pitch) + amount * (octaveMultiplier ? TheScale->NumTonesInScale() : 1));
      case LaunchpadLayout::kDrum:
         return pitch + amount * (octaveMultiplier ? 4 : 1);
      case LaunchpadLayout::kPentatonic:
      {
         int direction = amount > 0 ? 1 : -1;
         int transpose = amount * (octaveMultiplier ? 5 : 1) * direction;
         for (int i = 0; i < transpose; ++i)
         {
            do
            {
               pitch += direction;
            } while (pitch >= 0 && pitch < 127 && !TheScale->IsInPentatonic(pitch));
         }
         return pitch;
      }
   }
   return pitch + amount * (octaveMultiplier ? TheScale->GetPitchesPerOctave() : 1);
}

void LaunchpadKeyboard::UpdateLights(bool force)
{
   if (mGridControlTarget->GetGridController())
   {
      mCols = mGridControlTarget->GetGridController()->NumCols();
      mRows = mGridControlTarget->GetGridController()->NumRows();
      for (int x = 0; x < mCols; ++x)
      {
         for (int y = 0; y < mRows; ++y)
         {
            GridColor color = GetGridSquareColor(x, y);
            mGridControlTarget->GetGridController()->SetLight(x, y, color, force);
         }
      }
   }
}

GridColor LaunchpadKeyboard::GetGridSquareColor(int x, int y)
{
   int pitch = GridToPitch(x, y);
   bool inScale = TheScale->MakeDiatonic(pitch) == pitch;
   bool isRoot = pitch % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot();
   bool isHeld = GetHeldVelocity(GridToPitch(x, y)) > 0;
   bool isInPentatonic = pitch >= 0 && TheScale->IsInPentatonic(pitch);
   bool isChordButton = pitch != INVALID_PITCH && pitch < 0;
   bool isPressedChordButton = isChordButton && IsChordButtonPressed(pitch);
   bool isChorderEnabled = mChorder && mChorder->IsEnabled();
   bool isPreviewNote = pitch >= 0 && pitch < 128 && mPreviewNotes[pitch];

   GridColor color;
   if (pitch == INVALID_PITCH)
   {
      color = kGridColorOff;
   }
   else if (pitch == CHORD_ENABLE_BUTTON)
   {
      if (mChorder && mChorder->IsEnabled())
         color = kGridColor1Bright;
      else
         color = kGridColor1Dim;
   }
   else if (pitch == CHORD_LATCH_BUTTON)
   {
      if (mLatchChords)
         color = kGridColor3Bright;
      else
         color = kGridColor3Dim;
   }
   else if (pitch == KEY_LATCH_BUTTON)
   {
      if (mLatch)
         color = kGridColor1Bright;
      else
         color = kGridColor1Dim;
   }
   else if (isPreviewNote)
   {
      color = kGridColor1Bright;
   }
   else if (isPressedChordButton && isChorderEnabled)
   {
      color = kGridColor2Bright;
   }
   else if (isChordButton && !isChorderEnabled)
   {
      color = kGridColor1Dim;
   }
   else if (isChordButton)
   {
      color = kGridColor2Dim;
   }
   else if (isHeld)
   {
      color = kGridColor1Bright;
   }
   else if (mLayout == LaunchpadLayout::kDrum || mLayout == LaunchpadLayout::kAllPads)
   {
      color = kGridColor3Bright;
   }
   else if (mLayout == LaunchpadLayout::kPiano)
   {
      if (y % 2 == 0) //black keys
         color = kGridColor2Bright;
      else //white keys
         color = kGridColor3Bright;
   }
   else if (isRoot)
   {
      color = kGridColor2Bright;
   }
   else if (isInPentatonic)
   {
      color = kGridColor3Bright;
   }
   else if (inScale)
   {
      color = kGridColor3Dim;
   }
   else
   {
      color = kGridColorOff;
   }

   return color;
}

void LaunchpadKeyboard::OnControllerPageSelected()
{
   UpdateLights();
}

void LaunchpadKeyboard::OnScaleChanged()
{
   UpdateLights();
}

void LaunchpadKeyboard::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
}

void LaunchpadKeyboard::KeyReleased(int key)
{
   IDrawableModule::KeyReleased(key);
}

void LaunchpadKeyboard::Poll()
{
   bool chorderEnabled = mChorder && mChorder->IsEnabled();
   if (chorderEnabled != mWasChorderEnabled)
   {
      mWasChorderEnabled = chorderEnabled;
      UpdateLights();
   }
}

void LaunchpadKeyboard::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mHeldChordTones.clear();
      mNoteOutput.Flush(time);
   }
   if (checkbox == mPreserveChordRootCheckbox)
   {
      if (!ListContains(0, mHeldChordTones))
      {
         mHeldChordTones.push_back(0);
         if (mChorder)
            mChorder->AddTone(0);
         UpdateLights();
      }
   }
   if (checkbox == mLatchCheckbox)
   {
      if (!mLatch)
      {
         mNoteOutput.Flush(time);
      }
   }
}

void LaunchpadKeyboard::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mOctaveSlider)
   {
      for (int i = 0; i < 128; ++i)
         mCurrentNotes[i] = 0;
      mNoteOutput.Flush(time);
      UpdateLights();
   }
}

void LaunchpadKeyboard::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void LaunchpadKeyboard::Exit()
{
   IDrawableModule::Exit();
   if (mGridControlTarget->GetGridController())
      mGridControlTarget->GetGridController()->ResetLights();
}

void LaunchpadKeyboard::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   UpdateLights();
}

void LaunchpadKeyboard::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("chorder", moduleInfo, "", FillDropdown<Chorder*>);
   mModuleSaveData.LoadEnum<ArrangementMode>("arrangement", moduleInfo, (int)ArrangementMode::kFull, mArrangementModeDropdown);
   mModuleSaveData.LoadEnum<LaunchpadLayout>("layout", moduleInfo, (int)LaunchpadLayout::kChromatic, mLayoutDropdown);

   SetUpFromSaveData();
}

void LaunchpadKeyboard::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["chorder"] = mChorder ? mChorder->Path() : "";
}

void LaunchpadKeyboard::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetChorder(dynamic_cast<Chorder*>(TheSynth->FindModule(mModuleSaveData.GetString("chorder"), false)));
   mArrangementMode = mModuleSaveData.GetEnum<ArrangementMode>("arrangement");
   mLayout = mModuleSaveData.GetEnum<LaunchpadLayout>("layout");
}

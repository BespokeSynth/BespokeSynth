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
//  Chorder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Chorder.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PolyphonyMgr.h"
#include "ChordDatabase.h"

#include <cstring>

Chorder::Chorder()
{
   std::memset(mHeldCount, 0, TOTAL_NUM_NOTES * sizeof(int));
   std::memset(mInputNotes, 0, TOTAL_NUM_NOTES * sizeof(bool));

   TheScale->AddListener(this);
}

Chorder::~Chorder()
{
   TheScale->RemoveListener(this);
}

void Chorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChordGrid = new UIGrid(this, "uigrid", 2, 2, 130, 50, mDiatonic ? TheScale->NumTonesInScale() : TheScale->GetPitchesPerOctave(), 3);

   mChordGrid->SetVal(0, 1, 1);
   mChordGrid->SetListener(this);

   mDiatonicCheckbox = new Checkbox(this, "diatonic", mChordGrid, kAnchor_Below, &mDiatonic);
   mChordDropdown = new DropdownList(this, "chord", mDiatonicCheckbox, kAnchor_Right, &mChordIndex, 40);
   mInversionDropdown = new DropdownList(this, "inversion", mChordDropdown, kAnchor_Right, &mInversion, 30);

   for (auto name : TheScale->GetChordDatabase().GetChordNames())
      mChordDropdown->AddLabel(name, mChordDropdown->GetNumValues());

   mInversionDropdown->AddLabel("0", 0);
   mInversionDropdown->AddLabel("1", 1);
   mInversionDropdown->AddLabel("2", 2);
   mInversionDropdown->AddLabel("3", 3);
}

void Chorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofFill();
   for (int i = 0; i < mChordGrid->GetCols(); ++i)
   {
      bool addColor = false;
      if (i == 0)
      {
         ofSetColor(0, 255, 0, 80);
         addColor = true;
      }
      else if ((mDiatonic && i == 4 && TheScale->NumTonesInScale() == 7) || (!mDiatonic && i == 7 && TheScale->GetPitchesPerOctave() == 12))
      {
         ofSetColor(200, 150, 0, 80);
         addColor = true;
      }

      if (addColor)
      {
         ofRectangle rect = mChordGrid->GetRect(true);
         rect.width /= mChordGrid->GetCols();
         rect.x += i * rect.width;
         ofRect(rect);
      }
   }
   ofPopStyle();

   mChordGrid->Draw();
   mDiatonicCheckbox->Draw();
   mChordDropdown->Draw();
   mInversionDropdown->Draw();
}

void Chorder::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (!mDiatonic)
      return; //TODO(Ryan)

   int tone = col + (mChordGrid->GetRows() / 2 - row) * mChordGrid->GetCols();
   if (value > 0 && oldValue == 0)
   {
      AddTone(tone, value * value);
   }
   else if (value == 0 && oldValue > 0)
   {
      RemoveTone(tone);
   }
}

void Chorder::AddTone(int tone, float velocity)
{
   mChordGrid->SetVal((tone + mChordGrid->GetCols() * 10) % mChordGrid->GetCols(),
                      mChordGrid->GetRows() - 1 - (tone + (mChordGrid->GetRows() / 2 * mChordGrid->GetCols())) / mChordGrid->GetCols(),
                      sqrtf(velocity), !K(notifyListeners));
   for (int i = 0; i < TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         int chordtone = tone + TheScale->GetToneFromPitch(i);
         int outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(chordtone));
         PlayChorderNote(NoteMessage(NextBufferTime(false), outPitch, mVelocity * velocity));
      }
   }
}

void Chorder::RemoveTone(int tone)
{
   mChordGrid->SetVal((tone + mChordGrid->GetCols() * 10) % mChordGrid->GetCols(),
                      mChordGrid->GetRows() - 1 - (tone + (mChordGrid->GetRows() / 2 * mChordGrid->GetCols())) / mChordGrid->GetCols(),
                      0, !K(notifyListeners));
   for (int i = 0; i < TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         int chordtone = tone + TheScale->GetToneFromPitch(i);
         int outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(chordtone));
         PlayChorderNote(NoteMessage(NextBufferTime(false), outPitch, 0));
      }
   }
}

void Chorder::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   mChordGrid->TestClick(x, y, right);
}

void Chorder::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mChordGrid->MouseReleased();
}

bool Chorder::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mChordGrid->NotifyMouseMoved(x, y);
   return false;
}

void Chorder::OnScaleChanged()
{
   mChordGrid->SetGrid(mDiatonic ? TheScale->NumTonesInScale() : TheScale->GetPitchesPerOctave(), 3);
}

void Chorder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
      std::memset(mHeldCount, 0, TOTAL_NUM_NOTES * sizeof(int));
      std::memset(mInputNotes, 0, TOTAL_NUM_NOTES * sizeof(bool));
   }

   if (checkbox == mDiatonicCheckbox)
   {
      mChordDropdown->SetShowing(!mDiatonic);
      mInversionDropdown->SetShowing(!mDiatonic);
      mChordGrid->SetGrid(mDiatonic ? TheScale->NumTonesInScale() : TheScale->GetPitchesPerOctave(), 3);
   }
}

void Chorder::DropdownUpdated(DropdownList* dropdown, int oldVal, double time)
{
   if (dropdown == mChordDropdown || dropdown == mInversionDropdown)
   {
      std::vector<int> chord = TheScale->GetChordDatabase().GetChord(mChordDropdown->GetLabel(mChordIndex), mInversion);
      mChordGrid->Clear();
      for (int val : chord)
      {
         int row = 2 - ((val + TheScale->GetPitchesPerOctave()) / TheScale->GetPitchesPerOctave());
         int col = (val + TheScale->GetPitchesPerOctave()) % TheScale->GetPitchesPerOctave();
         mChordGrid->SetVal(col, row, 1);
      }
   }
}

void Chorder::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   bool noteOn = note.velocity > 0;
   if (mInputNotes[note.pitch] == noteOn)
      return;
   mInputNotes[note.pitch] = noteOn;

   if (note.velocity > 0)
      mVelocity = note.velocity;

   int idx = 0;
   // iterate rows from bottom to top to go from lowest to highest note
   // for compatibility with arpeggiator and strummer
   for (int row = mChordGrid->GetRows() - 1; row >= 0; --row)
   {
      for (int col = 0; col < mChordGrid->GetCols(); ++col)
      {
         float val = mChordGrid->GetVal(col, row);
         if (val > 0)
         {
            int gridPosition = col + (mChordGrid->GetRows() / 2 - row) * mChordGrid->GetCols();
            int voice = (note.voiceIdx == -1) ? -1 : (note.voiceIdx + idx) % 16;
            int outPitch;

            if (!mDiatonic)
            {
               outPitch = note.pitch + gridPosition;
            }
            else if (gridPosition % TheScale->NumTonesInScale() == 0) //if this is the pressed note or an octave of it
            {
               //play the pressed note (might not be in scale, so play it directly)
               int octave = gridPosition / TheScale->NumTonesInScale();
               outPitch = note.pitch + TheScale->GetPitchesPerOctave() * octave;
            }
            else
            {
               int tone = gridPosition + TheScale->GetToneFromPitch(note.pitch);
               outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(tone));
            }

            PlayChorderNote(NoteMessage(note.time, outPitch, note.velocity * val * val, voice, note.modulation));

            ++idx;
         }
      }
   }
   CheckLeftovers();
}

void Chorder::PlayChorderNote(NoteMessage note)
{
   assert(note.velocity >= 0);

   if (note.pitch < 0 || note.pitch >= TOTAL_NUM_NOTES)
      return;

   bool wasOn = mHeldCount[note.pitch] > 0;

   if (note.velocity > 0)
      ++mHeldCount[note.pitch];

   if (note.velocity == 0 && mHeldCount[note.pitch] > 0)
      --mHeldCount[note.pitch];

   if (mHeldCount[note.pitch] > 0 && !wasOn)
      PlayNoteOutput(note);
   if (mHeldCount[note.pitch] == 0 && wasOn)
      PlayNoteOutput(note.MakeNoteOff());

   //ofLog() << ofToString(pitch) + " " + ofToString(velocity) + ": " + ofToString(mHeldCount[pitch]) + " " + ofToString(voice);
}

void Chorder::CheckLeftovers()
{
   bool anyHeldNotes = false;
   for (int i = 0; i < TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         anyHeldNotes = true;
         break;
      }
   }

   if (!anyHeldNotes)
   {
      for (int i = 0; i < TOTAL_NUM_NOTES; ++i)
      {
         if (mHeldCount[i] > 0)
         {
            ofLog() << "Somehow there are still notes in the count! Clearing";
            PlayNoteOutput(NoteMessage(gTime, i, 0));
            mHeldCount[i] = 0;
         }
      }
   }
}

void Chorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("multislider_mode", moduleInfo, true);

   SetUpFromSaveData();
}

void Chorder::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));

   bool multisliderMode = mModuleSaveData.GetBool("multislider_mode");
   mChordGrid->SetGridMode(multisliderMode ? UIGrid::kMultisliderBipolar : UIGrid::kNormal);
   mChordGrid->SetRestrictDragToRow(multisliderMode);
   mChordGrid->SetRequireShiftForMultislider(true);
}

void Chorder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mChordGrid->SaveState(out);
}

void Chorder::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mChordGrid->LoadState(in);
}

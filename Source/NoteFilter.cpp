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
//  NoteFilter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#include "NoteFilter.h"
#include "SynthGlobals.h"

NoteFilter::NoteFilter()
{
   for (int i = 0; i < 128; ++i)
   {
      mGate[i] = true;
      mLastPlayTime[i] = -999;
   }
}

NoteFilter::~NoteFilter()
{
}

void NoteFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteFilter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int pitch = mMinPitch;
   for (auto* checkbox : mGateCheckboxes)
   {
      checkbox->Draw();
      ofPushStyle();
      ofFill();
      ofSetColor(0, 255, 0, (1 - ofClamp((gTime - mLastPlayTime[pitch]) / 250, 0, 1)) * 255);
      ofRect(75, checkbox->GetPosition(true).y + 4, 8, 8);
      ofPopStyle();
      ++pitch;
   }
}

void NoteFilter::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      if (note.pitch >= 0 && note.pitch < 128)
      {
         if (note.velocity > 0)
            mLastPlayTime[note.pitch] = note.time;
         if ((note.pitch >= mMinPitch && note.pitch <= mMaxPitch && mGate[note.pitch]) || note.velocity == 0)
            PlayNoteOutput(note);
      }
   }
   else
   {
      PlayNoteOutput(note);
   }
}

void NoteFilter::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 3 + (mMaxPitch - mMinPitch + 1) * 18;
}

void NoteFilter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("min pitch", moduleInfo, 0, 0, 127, K(isTextField));
   mModuleSaveData.LoadInt("max pitch", moduleInfo, 7, 0, 127, K(isTextField));

   SetUpFromSaveData();
}

void NoteFilter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));

   mMinPitch = mModuleSaveData.GetInt("min pitch");
   mMaxPitch = mModuleSaveData.GetInt("max pitch");

   for (auto* checkbox : mGateCheckboxes)
      RemoveUIControl(checkbox);
   mGateCheckboxes.clear();

   int numCheckboxes = (mMaxPitch - mMinPitch + 1);
   for (int i = 0; i < numCheckboxes; ++i)
   {
      int pitch = i + mMinPitch;
      Checkbox* checkbox = new Checkbox(this, (NoteName(pitch) + ofToString(pitch / 12 - 2) + " (" + ofToString(pitch) + ")").c_str(), 3, 3 + i * 18, &mGate[pitch]);
      mGateCheckboxes.push_back(checkbox);
   }
}

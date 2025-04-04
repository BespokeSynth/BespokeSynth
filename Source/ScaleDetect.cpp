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
//  ScaleDetect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/10/13.
//
//

#include "ScaleDetect.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

ScaleDetect::ScaleDetect()
{
}

void ScaleDetect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mResetButton = new ClickButton(this, "reset", 4, 18);
   mMatchesDropdown = new DropdownList(this, "matches", 25, 2, &mSelectedMatch);
}

void ScaleDetect::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mResetButton->Draw();
   mMatchesDropdown->Draw();

   DrawTextNormal(NoteName(mLastPitch), 5, 12);

   if (mNeedsUpdate)
   {
      mMatchesDropdown->Clear();
      int numMatches = 0;
      mSelectedMatch = 0;

      if (mDoDetect)
      {
         int numScaleTypes = TheScale->GetNumScaleTypes();
         for (int j = 0; j < numScaleTypes - 1; ++j)
         {
            if (ScaleSatisfied(mLastPitch % TheScale->GetPitchesPerOctave(), TheScale->GetScaleName(j)))
               mMatchesDropdown->AddLabel(TheScale->GetScaleName(j).c_str(), numMatches++);
         }
      }

      mNeedsUpdate = false;
   }

   {
      std::string pitchString;
      std::vector<int> rootRelative;
      for (int i = 0; i < 128; ++i)
      {
         if (mPitchOn[i])
         {
            int entry = (i - mLastPitch + TheScale->GetPitchesPerOctave() * 10) % TheScale->GetPitchesPerOctave();
            if (!VectorContains(entry, rootRelative))
               rootRelative.push_back(entry);
         }
      }
      sort(rootRelative.begin(), rootRelative.end());
      for (int i = 0; i < rootRelative.size(); ++i)
         pitchString += ofToString(rootRelative[i]) + " ";
      DrawTextNormal(pitchString, 40, 30);
   }
}

void ScaleDetect::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (note.velocity > 0 && note.pitch >= 0 && note.pitch < 128)
   {
      mPitchOn[note.pitch] = true;
      mLastPitch = note.pitch;
      mNeedsUpdate = true;
   }
}

bool ScaleDetect::ScaleSatisfied(int root, std::string type)
{
   ScalePitches scale;
   scale.SetRoot(root);
   scale.SetScaleType(type);

   for (int i = 0; i < 128; ++i)
   {
      if (mPitchOn[i] && !scale.IsInScale(i))
         return false;
   }
   return true;
}

void ScaleDetect::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
   {
      for (int i = 0; i < 128; ++i)
         mPitchOn[i] = false;
      mMatchesDropdown->Clear();
      mNeedsUpdate = true;
   }
}

void ScaleDetect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mMatchesDropdown)
   {
      TheScale->SetScale(mLastPitch, mMatchesDropdown->GetLabel(mSelectedMatch));
   }
}

void ScaleDetect::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ScaleDetect::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

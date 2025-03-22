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
//  WhiteKeys.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "WhiteKeys.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

WhiteKeys::WhiteKeys()
{
}

void WhiteKeys::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
}

void WhiteKeys::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void WhiteKeys::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   int octave = note.pitch / TheScale->GetPitchesPerOctave();
   int degree = -1;
   switch (note.pitch % TheScale->GetPitchesPerOctave())
   {
      default:
      case 0: degree = 0; break;
      case 2: degree = 1; break;
      case 4: degree = 2; break;
      case 5: degree = 3; break;
      case 7: degree = 4; break;
      case 9: degree = 5; break;
      case 11: degree = 6; break;
   }

   if (degree != -1)
   {
      note.pitch = TheScale->GetPitchFromTone(degree);
      note.pitch += octave * TheScale->GetPitchesPerOctave();
      PlayNoteOutput(note);
   }
}

void WhiteKeys::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void WhiteKeys::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

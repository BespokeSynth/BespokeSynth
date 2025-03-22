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
/*
  ==============================================================================

    NoteLatch.cpp
    Created: 11 Apr 2020 3:28:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteLatch.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteLatch::NoteLatch()
{
}

void NoteLatch::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteLatch::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < 128; ++i)
      {
         if (mNoteState[i])
         {
            PlayNoteOutput(NoteMessage(time, i, 0));
            mNoteState[i] = false;
         }
      }
   }
}

void NoteLatch::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      if (note.velocity > 0)
      {
         if (!mNoteState[note.pitch])
            PlayNoteOutput(note);
         else
            PlayNoteOutput(note.MakeNoteOff());
         mNoteState[note.pitch] = !mNoteState[note.pitch];
      }
   }
   else
   {
      PlayNoteOutput(note);
   }
}

void NoteLatch::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteLatch::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

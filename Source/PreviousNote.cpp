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
//  PreviousNote.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "PreviousNote.h"
#include "SynthGlobals.h"

PreviousNote::PreviousNote()
{
}

void PreviousNote::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PreviousNote::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity > 0)
   {
      int newPitch = note.pitch;
      int newVelocity = note.velocity;

      if (mPitch != -1)
      {
         note.pitch = mPitch;
         note.velocity = mVelocity;
         PlayNoteOutput(note);
      }

      mPitch = newPitch;
      mVelocity = newVelocity;
   }
   else
   {
      mNoteOutput.Flush(note.time);
   }
}

void PreviousNote::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PreviousNote::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

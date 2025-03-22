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
//  NoteDisplayer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#include "NoteDisplayer.h"
#include "SynthGlobals.h"

void NoteDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   bool* notes = mNoteOutput.GetNotes();
   float y = 14;
   for (int i = 0; i < 128; ++i)
   {
      if (notes[i])
      {
         DrawNoteName(i, y);
         y += 13;
      }
   }
}

void NoteDisplayer::DrawNoteName(int pitch, float y) const
{
   DrawTextNormal(NoteName(pitch) + ofToString(pitch / 12 - 2) + " (" + ofToString(pitch) + ")" +
                  " vel:" + ofToString(mVelocities[pitch]) +
                  " voiceId:" + ofToString(mVoiceIds[pitch]),
                  4, y);
}

void NoteDisplayer::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);
   mVelocities[note.pitch] = note.velocity;
   mVoiceIds[note.pitch] = note.voiceIdx;
}

void NoteDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void NoteDisplayer::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}

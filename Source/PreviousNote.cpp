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

void PreviousNote::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (velocity > 0)
   {
      if (mPitch != -1)
      {
         PlayNoteOutput(time, mPitch, mVelocity, voiceIdx, modulation);
      }

      mPitch = pitch;
      mVelocity = velocity;
   }
   else
   {
      mNoteOutput.Flush(time);
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

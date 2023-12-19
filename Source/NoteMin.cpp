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

    ChordHold.cpp
    Created: 3 Mar 2021 9:56:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteMin.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteMin::NoteMin()
{
}

void NoteMin::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteMin::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteMin::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      if (velocity > 0)
      {
         bool anyInputNotesHeld = false;
         for (int i = 0; i < 128; ++i)
         {
            if (mNoteInputHeld[i])
               anyInputNotesHeld = true;
         }

         if (!anyInputNotesHeld) //new input, clear any existing output
         {
            for (int i = 0; i < 128; ++i)
            {
               if (mNotePlaying[i])
               {
                  PlayNoteOutput(time, i, 0, -1);
                  mNotePlaying[i] = false;
               }
            }
         }

         if (!mOnlyPlayWhenPulsed)
         {
            if (!mNotePlaying[pitch]) //don't replay already-sustained notes
               PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
            mNotePlaying[pitch] = true;

            //stop playing any voices in the chord that aren't being held anymore
            for (int i = 0; i < 128; ++i)
            {
               if (i != pitch && mNotePlaying[i] && !mNoteInputHeld[i])
               {
                  PlayNoteOutput(time, i, 0, -1);
                  mNotePlaying[i] = false;
               }
            }
         }
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }

   mNoteInputHeld[pitch] = velocity > 0;
}

void NoteMin::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteMin::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

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
      mNotePlaying[pitch] = velocity > 0;
      int minNotePlaying = -1;
      for (int i = 0; i < 128; ++i)
      {
         if (mNotePlaying[i] && minNotePlaying == -1)
            minNotePlaying = i;
      }

      if (velocity > 0) // new note playing
      {
          if (minNotePlaying == pitch)
             PlayNoteOutput(time, minNotePlaying, velocity, voiceIdx, modulation);
      } else { // played note is stopped
          PlayNoteOutput(time, pitch, 0, -1);
          if (minNotePlaying > pitch) // play the new lowest note
             PlayNoteOutput(time, minNotePlaying, velocity, voiceIdx, modulation);
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
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

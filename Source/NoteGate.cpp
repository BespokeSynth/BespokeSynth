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
//  NoteGate.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#include "NoteGate.h"
#include "SynthGlobals.h"

NoteGate::NoteGate()
{
}

NoteGate::~NoteGate()
{
}

void NoteGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mGateCheckbox = new Checkbox(this, "open", 3, 4, &mGate);
}

void NoteGate::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGateCheckbox->Draw();
}

void NoteGate::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mGate || (velocity == 0 && mActiveNotes[pitch].velocity > 0))
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

      mActiveNotes[pitch].velocity = velocity;
      mActiveNotes[pitch].voiceIdx = voiceIdx;
      mActiveNotes[pitch].modulation = modulation;
   }
}

void NoteGate::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mGateCheckbox)
   {
      if (!mGate)
      {
         for (int pitch = 0; pitch < 128; ++pitch)
         {
            if (mActiveNotes[pitch].velocity > 0)
            {
               PlayNoteOutput(time + gBufferSizeMs, pitch, 0, mActiveNotes[pitch].voiceIdx, mActiveNotes[pitch].modulation);
               mActiveNotes[pitch].velocity = 0;
            }
         }
      }
   }
}

void NoteGate::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 20;
}

void NoteGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteGate::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

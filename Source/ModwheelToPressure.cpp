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
//  ModwheelToPressure.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModwheelToPressure.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModwheelToPressure::ModwheelToPressure()
{
}

ModwheelToPressure::~ModwheelToPressure()
{
}

void ModwheelToPressure::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void ModwheelToPressure::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      note.modulation.pressure = note.modulation.modWheel;
      note.modulation.modWheel = nullptr;
   }
   PlayNoteOutput(note);
}

void ModwheelToPressure::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ModwheelToPressure::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

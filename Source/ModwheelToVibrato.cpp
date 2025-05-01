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
//  ModwheelToVibrato.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModwheelToVibrato.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModwheelToVibrato::ModwheelToVibrato()
{
}

ModwheelToVibrato::~ModwheelToVibrato()
{
}

void ModwheelToVibrato::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVibratoSlider = new FloatSlider(this, "vibrato", 3, 3, 90, 15, &mVibratoAmount, 0, 1);
   mIntervalSelector = new DropdownList(this, "vibinterval", 96, 3, (int*)(&mVibratoInterval));

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

void ModwheelToVibrato::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mVibratoSlider->Draw();
   mIntervalSelector->Draw();
}

void ModwheelToVibrato::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(note.voiceIdx)->AppendTo(note.modulation.pitchBend);
      mModulation.GetPitchBend(note.voiceIdx)->SetLFO(mVibratoInterval, mVibratoAmount);
      mModulation.GetPitchBend(note.voiceIdx)->MultiplyIn(note.modulation.modWheel);
      note.modulation.pitchBend = mModulation.GetPitchBend(note.voiceIdx);
      note.modulation.modWheel = nullptr;
   }

   PlayNoteOutput(note);
}

void ModwheelToVibrato::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void ModwheelToVibrato::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void ModwheelToVibrato::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void ModwheelToVibrato::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ModwheelToVibrato::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

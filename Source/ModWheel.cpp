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
//  ModWheel.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModWheel.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModWheel::ModWheel()
{
}

void ModWheel::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

ModWheel::~ModWheel()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModWheel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mModWheelSlider = new FloatSlider(this, "modwheel", 5, 2, 110, 15, &mModWheel, 0, 1);
}

void ModWheel::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mModWheelSlider->Draw();
}

void ModWheel::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      mModulation.GetModWheel(note.voiceIdx)->AppendTo(note.modulation.modWheel);
      note.modulation.modWheel = mModulation.GetModWheel(note.voiceIdx);
   }

   PlayNoteOutput(note);
}

void ModWheel::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void ModWheel::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mModWheelSlider)
      mModulation.GetModWheel(-1)->SetValue(mModWheel);
}

void ModWheel::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void ModWheel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ModWheel::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

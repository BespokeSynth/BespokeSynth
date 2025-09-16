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
//  Pressure.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "Pressure.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

Pressure::Pressure()
{
}

void Pressure::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

Pressure::~Pressure()
{
   TheTransport->RemoveAudioPoller(this);
}

void Pressure::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPressureSlider = new FloatSlider(this, "pressure", 5, 2, 110, 15, &mPressure, 0, 1);
}

void Pressure::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mPressureSlider->Draw();
}

void Pressure::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      mModulation.GetPressure(note.voiceIdx)->AppendTo(note.modulation.pressure);
      note.modulation.pressure = mModulation.GetPressure(note.voiceIdx);
   }

   PlayNoteOutput(note);
}

void Pressure::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void Pressure::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mPressureSlider)
      mModulation.GetPressure(-1)->SetValue(mPressure);
}

void Pressure::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void Pressure::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Pressure::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

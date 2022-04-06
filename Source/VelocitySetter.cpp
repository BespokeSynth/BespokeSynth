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
//  VelocitySetter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/16/13.
//
//

#include "VelocitySetter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

VelocitySetter::VelocitySetter()
: mVelocity(1)
, mVelocitySlider(nullptr)
, mRandomness(0)
, mRandomnessSlider(nullptr)
{
}

void VelocitySetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVelocitySlider = new FloatSlider(this, "vel", 5, 2, 80, 15, &mVelocity, 0, 1);
   mRandomnessSlider = new FloatSlider(this, "rand", 5, 20, 80, 15, &mRandomness, 0, 1);
}

void VelocitySetter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mVelocitySlider->Draw();
   mRandomnessSlider->Draw();
}

void VelocitySetter::CheckboxUpdated(Checkbox* checkbox)
{
}

void VelocitySetter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);

   float random = ofRandom(1 - mRandomness, 1);

   if (mEnabled && velocity != 0)
   {
      PlayNoteOutput(time, pitch, int(mVelocity * 127 * random), voiceIdx, modulation);
   }
   else
   {
      PlayNoteOutput(time, pitch, int(velocity * random), voiceIdx, modulation);
      if (velocity != 0)
         mVelocity = velocity / 127.0f;
   }
}

void VelocitySetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VelocitySetter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

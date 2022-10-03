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
//  PitchAssigner.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/27/15.
//
//

#include "PitchSetter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

PitchSetter::PitchSetter()
{
}

void PitchSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchSlider = new IntSlider(this, "pitch", 5, 2, 80, 15, &mPitch, 0, 127);
}

void PitchSetter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mPitchSlider->Draw();
}

void PitchSetter::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void PitchSetter::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mPitchSlider)
      mNoteOutput.Flush(time);
}

void PitchSetter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);

   if (mEnabled)
      PlayNoteOutput(time, mPitch, velocity, voiceIdx, modulation);
   else
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void PitchSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PitchSetter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

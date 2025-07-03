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

    ModulatorSubtract.cpp
    Created: 9 Dec 2019 10:11:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorSubtract.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorSubtract::ModulatorSubtract()
{
}

void ModulatorSubtract::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mValue1Slider = new FloatSlider(this, "value 1", 3, 2, 100, 15, &mValue1, 0, 1);
   mValue2Slider = new FloatSlider(this, "value 2", mValue1Slider, kAnchor_Below, 100, 15, &mValue2, 0, 1);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

ModulatorSubtract::~ModulatorSubtract()
{
}

void ModulatorSubtract::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mValue1Slider->Draw();
   mValue2Slider->Draw();
}

void ModulatorSubtract::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (GetSliderTarget() && fromUserClick)
   {
      mValue1 = GetSliderTarget()->GetValue();
      mValue2 = 0;
      mValue1Slider->SetExtents(GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
      mValue1Slider->SetMode(GetSliderTarget()->GetMode());
   }
}

float ModulatorSubtract::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   if (GetSliderTarget())
      return ofClamp(mValue1 - mValue2, GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
   else
      return mValue1 - mValue2;
}

void ModulatorSubtract::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorSubtract::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorSubtract::SetUpFromSaveData()
{
}

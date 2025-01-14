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

    PressureToCV.cpp
    Created: 28 Nov 2017 9:44:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PressureToCV.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PressureToCV::PressureToCV()
{
}

PressureToCV::~PressureToCV()
{
}

void PressureToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);

   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
}

void PressureToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void PressureToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PressureToCV::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      mPressure = note.modulation.pressure;
   }
}

float PressureToCV::Value(int samplesIn)
{
   float pressure = mPressure ? mPressure->GetValue(samplesIn) : ModulationParameters::kDefaultPressure;
   return ofMap(pressure, 0, 1, GetMin(), GetMax(), K(clamped));
}

void PressureToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void PressureToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void PressureToCV::SetUpFromSaveData()
{
}

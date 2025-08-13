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

    ModulatorBinaryValue.cpp
    Created: 13 Aug 2025 8:00:00am
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "ModulatorBinaryValue.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorBinaryValue::ModulatorBinaryValue()
{
}

void ModulatorBinaryValue::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mInputSlider = new FloatSlider(this, "input", 3, 2, 100, 15, &mInput, 0, 1);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

ModulatorBinaryValue::~ModulatorBinaryValue()
{
}

void ModulatorBinaryValue::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mInputSlider->Draw();
}

void ModulatorBinaryValue::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void ModulatorBinaryValue::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorBinaryValue::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorBinaryValue::SetUpFromSaveData()
{
}

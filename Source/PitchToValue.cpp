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
//  PitchToValue.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 12/20/23.
//
//

#include "PitchToValue.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PitchToValue::PitchToValue()
: mValue(0)
{
}

PitchToValue::~PitchToValue()
{
}

void PitchToValue::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

void PitchToValue::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PitchToValue::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PitchToValue::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mValue = pitch;
   }
}

float PitchToValue::Value(int samplesIn)
{
   return mValue;
}

void PitchToValue::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void PitchToValue::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void PitchToValue::SetUpFromSaveData()
{
}

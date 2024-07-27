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

    ModulatorAccum.cpp
    Created: 2 Aug 2021 10:32:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorAccum.h"

#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

ModulatorAccum::ModulatorAccum()
{
}

void ModulatorAccum::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void ModulatorAccum::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mValueSlider, "value", &mValue, 0, 1);
   FLOATSLIDER(mVelocitySlider, "velocity", &mVelocity, -1, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

ModulatorAccum::~ModulatorAccum()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorAccum::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mValueSlider->Draw();
   mVelocitySlider->Draw();
}

void ModulatorAccum::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (GetSliderTarget() && fromUserClick)
   {
      mValue = GetSliderTarget()->GetValue();
      mValueSlider->SetExtents(GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
      mValueSlider->SetMode(GetSliderTarget()->GetMode());
   }
}

void ModulatorAccum::OnTransportAdvanced(float amount)
{
   float dt = amount * TheTransport->MsPerBar();
   float newValue = ofClamp(mValue + mVelocity / 1000 * (GetMax() - GetMin()) * dt, GetMin(), GetMax());
   mValue = newValue;
}

float ModulatorAccum::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   float dt = samplesIn / gSampleRate;
   float value = ofClamp(mValue + mVelocity / 1000 * (GetMax() - GetMin()) * dt, GetMin(), GetMax());
   return value;
}

void ModulatorAccum::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorAccum::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorAccum::SetUpFromSaveData()
{
}

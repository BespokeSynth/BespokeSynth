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

    ModulatorSmoother.cpp
    Created: 29 Nov 2017 9:35:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorSmoother.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorSmoother::ModulatorSmoother()
{
}

void ModulatorSmoother::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void ModulatorSmoother::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mInputSlider = new FloatSlider(this, "input", 3, 2, 100, 15, &mInput, 0, 1);
   mSmoothSlider = new FloatSlider(this, "smooth", mInputSlider, kAnchor_Below, 100, 15, &mSmooth, 0, 1);
   mSmoothSlider->SetMode(FloatSlider::kSquare);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

ModulatorSmoother::~ModulatorSmoother()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorSmoother::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mInputSlider->Draw();
   mSmoothSlider->Draw();
}

void ModulatorSmoother::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (GetSliderTarget() && fromUserClick)
   {
      mInput = GetSliderTarget()->GetValue();
      mInputSlider->SetExtents(GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
      mInputSlider->SetMode(GetSliderTarget()->GetMode());
   }
}

void ModulatorSmoother::OnTransportAdvanced(float amount)
{
   mRamp.Start(gTime, mInput, gTime + (amount * TheTransport->MsPerBar() * (mSmooth * 300)));
}

float ModulatorSmoother::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mRamp.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax());
}

void ModulatorSmoother::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorSmoother::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorSmoother::SetUpFromSaveData()
{
}

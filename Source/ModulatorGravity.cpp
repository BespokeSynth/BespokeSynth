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

    ModulatorGravity.cpp
    Created: 30 Apr 2020 3:56:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorGravity.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

ModulatorGravity::ModulatorGravity()
{
}

void ModulatorGravity::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void ModulatorGravity::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mGravitySlider, "gravity", &mGravity, -1, 1);
   FLOATSLIDER(mKickAmountSlider, "kick amt", &mKickAmount, -5, 5);
   FLOATSLIDER(mDragSlider, "drag", &mDrag, 0, .01f);
   BUTTON(mKickButton, "kick");
   ENDUIBLOCK(mWidth, mHeight);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

ModulatorGravity::~ModulatorGravity()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorGravity::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGravitySlider->Draw();
   mKickAmountSlider->Draw();
   mDragSlider->Draw();
   mKickButton->Draw();
}

void ModulatorGravity::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void ModulatorGravity::OnTransportAdvanced(float amount)
{
   float dt = amount * TheTransport->MsPerBar();
   float newVelocity = mVelocity + mGravity / 100000 * dt;
   newVelocity -= newVelocity * mDrag * dt;
   float newValue = ofClamp(mValue + newVelocity * dt, 0, 1);
   mVelocity = (newValue - mValue) / dt;
   mValue = newValue;
}

float ModulatorGravity::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   //return ofClamp(mRamp.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax());
   return ofLerp(GetMin(), GetMax(), mValue); //TODO(integrate over samples)
}

void ModulatorGravity::OnPulse(double time, float velocity, int flags)
{
   Kick(velocity);
}

void ModulatorGravity::ButtonClicked(ClickButton* button, double time)
{
   if (button == mKickButton)
      Kick(1);
}

void ModulatorGravity::Kick(float strength)
{
   mVelocity += mKickAmount / 1000 * strength;
}

void ModulatorGravity::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorGravity::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorGravity::SetUpFromSaveData()
{
}

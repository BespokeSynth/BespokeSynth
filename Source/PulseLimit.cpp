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

    PulseLimit.cpp
    Created: 22 Oct 2024 11:32:00am
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "PulseLimit.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Transport.h"

PulseLimit::PulseLimit()
: mCount(0)
{
}

PulseLimit::~PulseLimit()
{
}

void PulseLimit::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   BUTTON(mResetButton, "reset");
   INTSLIDER(mLimitSlider, "limit", &mLimit, 1, 127);
   ENDUIBLOCK0();
}

void PulseLimit::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PulseLimit::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (!mEnabled)
   {
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
      return;
   }

   mCount++;
   if (mCount < mLimit)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PulseLimit::ButtonClicked(ClickButton* button, double time)
{
   mCount = 0;
}

void PulseLimit::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = mDeterministic ? 61 : 38;
}

void PulseLimit::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseLimit::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

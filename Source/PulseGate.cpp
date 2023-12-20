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

    PulseGate.cpp
    Created: 22 Feb 2020 10:39:40pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseGate.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Checkbox.h"

PulseGate::PulseGate()
{
}

PulseGate::~PulseGate()
{
}

void PulseGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mAllowCheckbox, "allow", &mAllow);
   ENDUIBLOCK(mWidth, mHeight);
}

void PulseGate::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAllowCheckbox->Draw();
}

void PulseGate::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (mAllow)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PulseGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseGate::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

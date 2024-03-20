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

    PulseFlag.cpp
    Created: 24 Jan 2023
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseFlag.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Transport.h"

PulseFlag::PulseFlag()
{
}

PulseFlag::~PulseFlag()
{
}

void PulseFlag::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mFlagValueSelector, "flag", &mFlagValue, 70);
   CHECKBOX(mReplaceFlagsCheckbox, "replace", &mReplaceFlags);
   ENDUIBLOCK(mWidth, mHeight);

   mFlagValueSelector->AddLabel("none", kPulseFlag_None);
   mFlagValueSelector->AddLabel("reset", kPulseFlag_Reset);
   mFlagValueSelector->AddLabel("random", kPulseFlag_Random);
   mFlagValueSelector->AddLabel("sync", kPulseFlag_SyncToTransport);
   mFlagValueSelector->AddLabel("backward", kPulseFlag_Backward);
   mFlagValueSelector->AddLabel("align", kPulseFlag_Align);
   mFlagValueSelector->AddLabel("repeat", kPulseFlag_Repeat);
}

void PulseFlag::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mFlagValueSelector->Draw();
   mReplaceFlagsCheckbox->Draw();
}

void PulseFlag::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (mEnabled)
   {
      if (mReplaceFlags)
         flags = 0;

      flags |= mFlagValue;
   }

   DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PulseFlag::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void PulseFlag::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseFlag::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

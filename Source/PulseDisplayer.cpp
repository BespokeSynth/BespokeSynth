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

    PulseDisplayer.cpp
    Created: 26 Jan 2023
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseDisplayer.h"
#include "SynthGlobals.h"
#include "Transport.h"

PulseDisplayer::PulseDisplayer()
{
}

PulseDisplayer::~PulseDisplayer()
{
}

void PulseDisplayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void PulseDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   float brightness = ofLerp(150, 255, 1 - ofClamp(gTime - mLastReceivedFlagTime, 0, 200) / 200.0f);
   ofPushStyle();
   ofSetColor(brightness, brightness, brightness);
   std::string output;
   if (mLastReceivedFlags == kPulseFlag_None)
      output = "none";
   if (mLastReceivedFlags & kPulseFlag_Reset)
      output += "reset ";
   if (mLastReceivedFlags & kPulseFlag_Random)
      output += "random ";
   if (mLastReceivedFlags & kPulseFlag_SyncToTransport)
      output += "sync ";
   if (mLastReceivedFlags & kPulseFlag_Backward)
      output += "backward ";
   if (mLastReceivedFlags & kPulseFlag_Align)
      output += "align ";
   if (mLastReceivedFlags & kPulseFlag_Repeat)
      output += "repeat ";
   DrawTextNormal(output, 5, 18);
   ofPopStyle();
}

void PulseDisplayer::OnPulse(double time, float velocity, int flags)
{
   mLastReceivedFlags = flags;
   mLastReceivedFlagTime = gTime;

   DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PulseDisplayer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void PulseDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

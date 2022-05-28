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

    PulseChance.cpp
    Created: 4 Feb 2020 12:17:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseChance.h"
#include "SynthGlobals.h"

PulseChance::PulseChance()
{
}

PulseChance::~PulseChance()
{
}

void PulseChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChanceSlider = new FloatSlider(this, "chance", 3, 2, 100, 15, &mChance, 0, 1);
}

void PulseChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mChanceSlider->Draw();

   if (gTime - mLastAcceptTime > 0 && gTime - mLastAcceptTime < 200)
   {
      ofPushStyle();
      ofSetColor(0, 255, 0, 255 * (1 - (gTime - mLastAcceptTime) / 200));
      ofFill();
      ofRect(106, 2, 10, 7);
      ofPopStyle();
   }

   if (gTime - mLastRejectTime > 0 && gTime - mLastRejectTime < 200)
   {
      ofPushStyle();
      ofSetColor(255, 0, 0, 255 * (1 - (gTime - mLastRejectTime) / 200));
      ofFill();
      ofRect(106, 9, 10, 7);
      ofPopStyle();
   }
}

void PulseChance::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   bool accept = ofRandom(1) <= mChance;
   if (accept)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);

   if (accept)
      mLastAcceptTime = gTime;
   else
      mLastRejectTime = gTime;
}

void PulseChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = 20;
}

void PulseChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

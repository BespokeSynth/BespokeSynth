/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
// Created by block on 8/6/2022.
//

#include "BoundsToPulse.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

BoundsToPulse::BoundsToPulse()
: mSlider(nullptr)
, mValue(0)
{
}

BoundsToPulse::~BoundsToPulse()
{
}

void BoundsToPulse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSlider = new FloatSlider(this, "input", 5, 4, 100, 15, &mValue, 0, 1);

   mMinCable = new PatchCableSource(this, kConnectionType_Pulse);
   mMaxCable = new PatchCableSource(this, kConnectionType_Pulse);
   AddPatchCableSource(mMinCable);
   AddPatchCableSource(mMaxCable);
}

void BoundsToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSlider->Draw();

   mMinCable->SetManualPosition(10, 30);
   mMaxCable->SetManualPosition(100, 30);
}

void BoundsToPulse::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (!mEnabled)
      return;

   if (slider == mSlider)
   {
      // send the pulse through our main source *and* the relevant cable

      if (mValue == slider->GetMin() && mValue < oldVal)
      {
         DispatchPulse(GetPatchCableSource(), gTime + gBufferSizeMs, 1.f, 0);
         DispatchPulse(mMinCable, gTime + gBufferSizeMs, 1.f, 0);
      }
      else if (mValue == slider->GetMax() && oldVal < mValue)
      {
         DispatchPulse(GetPatchCableSource(), gTime + gBufferSizeMs, 1.f, 0);
         DispatchPulse(mMaxCable, gTime + gBufferSizeMs, 1.f, 0);
      }
   }
}
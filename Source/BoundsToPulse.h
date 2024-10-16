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

#pragma once

#include "IAudioReceiver.h"
#include "INoteSource.h"
#include "Slider.h"
#include "IDrawableModule.h"
#include "RadioButton.h"
#include "BiquadFilter.h"
#include "Scale.h"
#include "IAudioSource.h"
#include "IPulseReceiver.h"
#include "IAudioPoller.h"

class BoundsToPulse : public IDrawableModule, public IFloatSliderListener, public IPulseSource, public IAudioPoller
{
public:
   BoundsToPulse();
   virtual ~BoundsToPulse();
   static IDrawableModule* Create() { return new BoundsToPulse(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void Init() override;
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnTransportAdvanced(float amount) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   float GetValue() const { return mValue; }
   FloatSlider* GetSlider() { return mSlider; }

private:
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 110;
      height = 40;
   }

   FloatSlider* mSlider;
   float mValue;

   PatchCableSource* mMinCable{};
   PatchCableSource* mMaxCable{};
};
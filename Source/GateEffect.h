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
//
//  GateEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/19/13.
//
//

#pragma once

#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class GateEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   GateEffect();
   static IAudioEffect* Create() { return new GateEffect(); }


   void CreateUIControls() override;

   void SetAttack(double ms) { mAttackTime = ms; }
   void SetRelease(double ms) { mReleaseTime = ms; }
   bool IsGateOpen() const { return mEnvelope > 0; }

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   std::string GetType() override { return "gate"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& width, double& height) override
   {
      width = 120;
      height = 50;
   }

   double mThreshold{ .1 };
   double mAttackTime{ 1 };
   double mReleaseTime{ 1 };
   FloatSlider* mThresholdSlider{ nullptr };
   FloatSlider* mAttackSlider{ nullptr };
   FloatSlider* mReleaseSlider{ nullptr };
   double mEnvelope{ 0 };
   double mPeak{ 0 };
};

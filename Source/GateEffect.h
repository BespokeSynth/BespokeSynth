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

#ifndef __modularSynth__GateEffect__
#define __modularSynth__GateEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class GateEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   GateEffect();
   static IAudioEffect* Create() { return new GateEffect(); }


   void CreateUIControls() override;

   void SetAttack(float ms) { mAttackTime = ms; }
   void SetRelease(float ms) { mReleaseTime = ms; }
   bool IsGateOpen() const { return mEnvelope > 0; }

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   std::string GetType() override { return "gate"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 50;
   }
   bool Enabled() const override { return mEnabled; }

   float mThreshold;
   float mAttackTime;
   float mReleaseTime;
   FloatSlider* mThresholdSlider;
   FloatSlider* mAttackSlider;
   FloatSlider* mReleaseSlider;
   float mEnvelope;
   float mPeak;
};

#endif /* defined(__modularSynth__GateEffect__) */

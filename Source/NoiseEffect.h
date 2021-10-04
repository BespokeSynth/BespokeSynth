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
//  NoiseEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#ifndef __modularSynth__NoiseEffect__
#define __modularSynth__NoiseEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class NoiseEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   NoiseEffect();
   
   static IAudioEffect* Create() { return new NoiseEffect(); }
   
   std::string GetTitleLabel() override { return "noisify"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "noisify"; }

   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=60; }
   bool Enabled() const override { return mEnabled; }

   
   float mAmount;
   int mWidth;
   int mSampleCounter;
   float mRandom;
   FloatSlider* mAmountSlider;
   IntSlider* mWidthSlider;
   
};

#endif /* defined(__modularSynth__NoiseEffect__) */


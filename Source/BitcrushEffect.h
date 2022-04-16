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
//  BitcrushEffect.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#ifndef __additiveSynth__BitcrushEffect__
#define __additiveSynth__BitcrushEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class BitcrushEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   BitcrushEffect();

   static IAudioEffect* Create() { return new BitcrushEffect(); }


   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "bitcrush"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   float mCrush;
   float mDownsample;
   int mSampleCounter[ChannelBuffer::kMaxNumChannels];
   float mHeldDownsample[ChannelBuffer::kMaxNumChannels];
   FloatSlider* mCrushSlider;
   FloatSlider* mDownsampleSlider;

   float mWidth;
   float mHeight;
};

#endif /* defined(__additiveSynth__BitcrushEffect__) */

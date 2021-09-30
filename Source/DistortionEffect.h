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
//  DistortionEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__DistortionEffect__
#define __modularSynth__DistortionEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "BiquadFilter.h"
#include "PeakTracker.h"

class DistortionEffect : public IAudioEffect, public IFloatSliderListener, public IDropdownListener
{
public:
   DistortionEffect();
   
   static IAudioEffect* Create() { return new DistortionEffect(); }
   
   std::string GetTitleLabel() override { return "distort"; }
   void CreateUIControls() override;
   
   void SetClip(float amount);
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "distortion"; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
private:
   enum DistortionType
   {
      kClean,
      kWarm,
      kDirty,
      kSoft,
      kAsymmetric,
      kFold,
      kGrungy
   };
   
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;
   
   DistortionType mType;
   float mClip;
   float mGain;
   float mPreamp;
   float mFuzzAmount;
   bool mRemoveInputDC;
   
   DropdownList* mTypeDropdown;
   FloatSlider* mClipSlider;
   FloatSlider* mPreampSlider;
   Checkbox* mRemoveInputDCCheckbox;
   FloatSlider* mFuzzAmountSlider;
   BiquadFilter mDCRemover[ChannelBuffer::kMaxNumChannels];
   PeakTracker mPeakTracker[ChannelBuffer::kMaxNumChannels];
};

#endif /* defined(__modularSynth__DistortionEffect__) */


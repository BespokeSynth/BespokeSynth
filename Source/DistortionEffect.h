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

#pragma once

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


   void CreateUIControls() override;

   void SetClip(float amount);

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "distortion"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   bool IsEnabled() const override { return mEnabled; }

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

   float mWidth{ 200 };
   float mHeight{ 20 };

   DistortionType mType{ DistortionType::kClean };
   float mClip{ 1 };
   float mGain{ 1 };
   float mPreamp{ 1 };
   float mFuzzAmount{ 0 };
   bool mRemoveInputDC{ true };

   DropdownList* mTypeDropdown{ nullptr };
   FloatSlider* mClipSlider{ nullptr };
   FloatSlider* mPreampSlider{ nullptr };
   Checkbox* mRemoveInputDCCheckbox{ nullptr };
   FloatSlider* mFuzzAmountSlider{ nullptr };
   BiquadFilter mDCRemover[ChannelBuffer::kMaxNumChannels]{};
   PeakTracker mPeakTracker[ChannelBuffer::kMaxNumChannels]{};
};

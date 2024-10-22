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
//  BiquadFilterEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/29/12.
//
//

#pragma once

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Slider.h"
#include "BiquadFilter.h"
#include "RadioButton.h"

class BiquadFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   BiquadFilterEffect();
   ~BiquadFilterEffect();

   static IAudioEffect* Create() { return new BiquadFilterEffect(); }


   void CreateUIControls() override;

   void Init() override;

   void SetFilterType(FilterType type) { mBiquad[0].SetFilterType(type); }
   void SetFilterParams(float f, float q) { mBiquad[0].SetFilterParams(f, q); }
   void Clear();

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "biquad"; }

   bool MouseMoved(float x, float y) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;

   void ResetFilter();

   RadioButton* mTypeSelector{ nullptr };

   FloatSlider* mFSlider{ nullptr };
   FloatSlider* mQSlider{ nullptr };
   FloatSlider* mGSlider{ nullptr };
   bool mMouseControl{ false };

   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels];
   ChannelBuffer mDryBuffer;

   bool mCoefficientsHaveChanged{ true };
};

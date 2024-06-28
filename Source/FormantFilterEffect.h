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
//  FormantFilterEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/21/16.
//
//

#pragma once

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "BiquadFilter.h"
#include "RadioButton.h"

class FormantFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   FormantFilterEffect();
   ~FormantFilterEffect();

   static IAudioEffect* Create() { return new FormantFilterEffect(); }


   void CreateUIControls() override;

   void Init() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "formant"; }

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
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void DrawModule() override;

   void ResetFilters();
   void UpdateFilters();

#define NUM_FORMANT_BANDS 3
   BiquadFilter mBiquads[NUM_FORMANT_BANDS];
   float* mDryBuffer{ nullptr };
   float mEE{ 1 };
   float mOO{ 0 };
   float mI{ 0 };
   float mE{ 0 };
   float mU{ 0 };
   float mA{ 0 };
   FloatSlider* mEESlider{ nullptr };
   FloatSlider* mOOSlider{ nullptr };
   FloatSlider* mISlider{ nullptr };
   FloatSlider* mESlider{ nullptr };
   FloatSlider* mUSlider{ nullptr };
   FloatSlider* mASlider{ nullptr };
   std::vector<FloatSlider*> mSliders{};
   bool mRescaling{ false };
   float mWidth{ 200 };
   float mHeight{ 20 };

   struct Formants
   {
      Formants(float f1, float g1, float f2, float g2, float f3, float g3)
      {
         assert(NUM_FORMANT_BANDS == 3);
         mFreqs[0] = f1;
         mGains[0] = g1;
         mFreqs[1] = f2;
         mGains[1] = g2;
         mFreqs[2] = f3;
         mGains[2] = g3;
      }
      float mFreqs[NUM_FORMANT_BANDS]{};
      float mGains[NUM_FORMANT_BANDS]{};
   };

   std::vector<Formants> mFormants;
   float* mOutputBuffer{ nullptr };
};

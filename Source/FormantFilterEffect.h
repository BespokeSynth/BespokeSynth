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

#ifndef __Bespoke__FormantFilter__
#define __Bespoke__FormantFilter__

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "BiquadFilter.h"
#include "RadioButton.h"

class FormantFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   FormantFilterEffect();
   ~FormantFilterEffect();
   
   static IAudioEffect* Create() { return new FormantFilterEffect(); }
   
   string GetTitleLabel() override { return "formant"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "formant"; }
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;
   
private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   void ResetFilters();
   void UpdateFilters();
   
#define NUM_FORMANT_BANDS 3
   BiquadFilter mBiquads[NUM_FORMANT_BANDS];
   float* mDryBuffer;
   int mDryBufferSize;
   float mEE;
   float mOO;
   float mI;
   float mE;
   float mU;
   float mA;
   FloatSlider* mEESlider;
   FloatSlider* mOOSlider;
   FloatSlider* mISlider;
   FloatSlider* mESlider;
   FloatSlider* mUSlider;
   FloatSlider* mASlider;
   vector<FloatSlider*> mSliders;
   bool mRescaling;
   float mWidth;
   float mHeight;
   
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
      float mFreqs[NUM_FORMANT_BANDS];
      float mGains[NUM_FORMANT_BANDS];
   };
   
   vector<Formants> mFormants;
   float* mOutputBuffer;
};

#endif /* defined(__Bespoke__FormantFilter__) */

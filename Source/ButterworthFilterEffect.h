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
//  ButterworthFilterEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__ButterworthFilterEffect__
#define __Bespoke__ButterworthFilterEffect__

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "FilterButterworth24db.h"

class ButterworthFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   ButterworthFilterEffect();
   ~ButterworthFilterEffect();

   static IAudioEffect* Create() { return new ButterworthFilterEffect(); }


   void CreateUIControls() override;

   void Init() override;

   void SetFilterParams(float f, float q) { mButterworth[0].Set(f, q); }

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "butterworth"; }

   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }

   void ResetFilter();

   float mF;
   FloatSlider* mFSlider;
   float mQ;
   FloatSlider* mQSlider;

   float mWidth;
   float mHeight;

   CFilterButterworth24db mButterworth[ChannelBuffer::kMaxNumChannels];
   ChannelBuffer mDryBuffer;

   bool mCoefficientsHaveChanged;
};

#endif /* defined(__Bespoke__ButterworthFilterEffect__) */

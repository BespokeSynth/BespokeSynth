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
//  FreeverbEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/14.
//
//

#ifndef __Bespoke__FreeverbEffect__
#define __Bespoke__FreeverbEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"
#include "freeverb/revmodel.hpp"

class FreeverbEffect : public IAudioEffect, public IFloatSliderListener
{
public:
   FreeverbEffect();
   ~FreeverbEffect();
   
   static IAudioEffect* Create() { return new FreeverbEffect(); }
   
   string GetTitleLabel() override { return "freeverb"; }
   void CreateUIControls() override;
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "freeverb"; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   revmodel mFreeverb;
   bool mNeedUpdate;
   bool mFreeze;
   float mRoomSize;
   float mDamp;
   float mWet;
   float mDry;
   float mVerbWidth;
   FloatSlider* mRoomSizeSlider;
   FloatSlider* mDampSlider;
   FloatSlider* mWetSlider;
   FloatSlider* mDrySlider;
   FloatSlider* mWidthSlider;
};

#endif /* defined(__Bespoke__FreeverbEffect__) */

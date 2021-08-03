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
//  PitchShiftEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#ifndef __Bespoke__PitchShiftEffect__
#define __Bespoke__PitchShiftEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "PitchShifter.h"
#include "RadioButton.h"

class PitchShiftEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   PitchShiftEffect();
   ~PitchShiftEffect();
   
   static IAudioEffect* Create() { return new PitchShiftEffect(); }
   
   string GetTitleLabel() override { return "pitchshift"; }
   void CreateUIControls() override;
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "pitchshift"; }
   
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float mRatio;
   FloatSlider* mRatioSlider;
   int mRatioSelection;
   RadioButton* mRatioSelector;
   PitchShifter* mPitchShifter[ChannelBuffer::kMaxNumChannels];
};


#endif /* defined(__Bespoke__PitchShiftEffect__) */

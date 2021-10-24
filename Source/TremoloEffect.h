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
//  TremoloEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#ifndef __modularSynth__TremoloEffect__
#define __modularSynth__TremoloEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"
#include "LFO.h"
#include "DropdownList.h"

class TremoloEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   TremoloEffect();
   
   static IAudioEffect* Create() { return new TremoloEffect(); }
   
   
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "tremolo"; }

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }

   float mAmount;
   FloatSlider* mAmountSlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   
   LFO mLFO;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   OscillatorType mOscType;
   DropdownList* mOscSelector;
   FloatSlider* mDutySlider;
   float mDuty;
   static const int kAntiPopWindowSize = 300;
   float mWindow[kAntiPopWindowSize];
   int mWindowPos;
   float mWidth;
   float mHeight;
};


#endif /* defined(__modularSynth__TremoloEffect__) */


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

#pragma once

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
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mAmount{ 0 };
   FloatSlider* mAmountSlider{ nullptr };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };

   LFO mLFO;
   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   OscillatorType mOscType{ OscillatorType::kOsc_Square };
   DropdownList* mOscSelector{ nullptr };
   FloatSlider* mDutySlider{ nullptr };
   float mDuty{ .5 };
   static const int kAntiPopWindowSize = 300;
   float mWindow[kAntiPopWindowSize]{};
   int mWindowPos{ 0 };
   float mWidth{ 200 };
   float mHeight{ 20 };
};

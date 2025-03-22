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
//  LiveGranulator.h
//  modularSynth
//
//  Created by Ryan Challinor on 10/2/13.
//
//

#pragma once

#include "IAudioEffect.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Granulator.h"
#include "Slider.h"
#include "RollingBuffer.h"
#include "Transport.h"
#include "DropdownList.h"

#define FREEZE_EXTRA_SAMPLES_COUNT 2 * gSampleRate

class LiveGranulator : public IAudioEffect, public IFloatSliderListener, public ITimeListener, public IDropdownListener
{
public:
   LiveGranulator();
   virtual ~LiveGranulator();

   static IAudioEffect* Create() { return new LiveGranulator(); }


   void CreateUIControls() override;
   void Init() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "granulator"; }

   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void Freeze();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mBufferLength;
   RollingBuffer mBuffer;
   Granulator mGranulator;
   FloatSlider* mGranOverlap{ nullptr };
   FloatSlider* mGranSpeed{ nullptr };
   FloatSlider* mGranLengthMs{ nullptr };
   FloatSlider* mGranPosRandomize{ nullptr };
   FloatSlider* mGranSpeedRandomize{ nullptr };
   FloatSlider* mGranSpacingRandomize{ nullptr };
   Checkbox* mGranOctaveCheckbox{ nullptr };
   float mDry{ 0 };
   FloatSlider* mDrySlider{ nullptr };
   bool mFreeze{ false };
   Checkbox* mFreezeCheckbox{ nullptr };
   int mFreezeExtraSamples{ 0 };
   float mPos{ 0 };
   FloatSlider* mPosSlider{ nullptr };
   NoteInterval mAutoCaptureInterval{ NoteInterval::kInterval_None };
   DropdownList* mAutoCaptureDropdown{ nullptr };
   FloatSlider* mWidthSlider{ nullptr };

   float mWidth{ 200 };
   float mHeight{ 20 };
   float mBufferX{ 0 };
};

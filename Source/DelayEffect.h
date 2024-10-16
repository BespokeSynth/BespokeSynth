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
//  DelayEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#pragma once

#include "IAudioEffect.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "Transport.h"
#include "Ramp.h"

#define DELAY_BUFFER_SIZE 5 * gSampleRate

class DelayEffect : public IAudioEffect, public IFloatSliderListener, public IDropdownListener
{
public:
   DelayEffect();

   static IAudioEffect* Create() { return new DelayEffect(); }


   void CreateUIControls() override;
   bool IsEnabled() const override { return mEnabled; }

   void SetDelay(float delay);
   void SetShortMode(bool on);
   void SetFeedback(float feedback) { mFeedback = feedback; }
   void Clear() { mDelayBuffer.ClearBuffer(); }
   void SetDry(bool dry) { mDry = dry; }
   void SetFeedbackModuleMode();

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override;
   float GetEffectAmount() override;
   std::string GetType() override { return "delay"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void DrawModule() override;

   float GetMinDelayMs() const;

   float mDelay{ 500 };
   float mFeedback{ 0 };
   bool mEcho{ true };
   RollingBuffer mDelayBuffer;
   FloatSlider* mFeedbackSlider{ nullptr };
   FloatSlider* mDelaySlider{ nullptr };
   Checkbox* mEchoCheckbox{ nullptr };
   NoteInterval mInterval{ NoteInterval::kInterval_8nd };
   DropdownList* mIntervalSelector{ nullptr };

   bool mShortTime{ false };
   Checkbox* mShortTimeCheckbox{ nullptr };
   Ramp mDelayRamp;
   Ramp mAmountRamp;
   bool mAcceptInput{ true };
   bool mDry{ true };
   bool mInvert{ false };
   Checkbox* mDryCheckbox{ nullptr };
   Checkbox* mAcceptInputCheckbox{ nullptr };
   Checkbox* mInvertCheckbox{ nullptr };

   float mWidth{ 200 };
   float mHeight{ 20 };

   bool mFeedbackModuleMode{ false }; //special mode when this delay effect is being used in a FeedbackModule
};

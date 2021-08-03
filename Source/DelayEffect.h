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

#ifndef __modularSynth__DelayEffect__
#define __modularSynth__DelayEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "Transport.h"
#include "Ramp.h"

#define DELAY_BUFFER_SIZE 5*gSampleRate

class DelayEffect : public IAudioEffect, public IFloatSliderListener, public IDropdownListener
{
public:
   DelayEffect();
   
   static IAudioEffect* Create() { return new DelayEffect(); }
   
   string GetTitleLabel() override { return "delay"; }
   void CreateUIControls() override;
   bool Enabled() const override { return mEnabled; }

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
   string GetType() override { return "delay"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void DrawModule() override;
   
   float GetMinDelayMs() const;
   
   float mDelay;
   float mFeedback;
   bool mEcho;
   RollingBuffer mDelayBuffer;
   FloatSlider* mFeedbackSlider;
   FloatSlider* mDelaySlider;
   Checkbox* mEchoCheckbox;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   
   bool mShortTime;
   Checkbox* mShortTimeCheckbox;
   Ramp mDelayRamp;
   Ramp mAmountRamp;
   bool mAcceptInput;
   bool mDry;
   bool mInvert;
   Checkbox* mDryCheckbox;
   Checkbox* mAcceptInputCheckbox;
   Checkbox* mInvertCheckbox;
   
   float mWidth;
   float mHeight;
   
   bool mFeedbackModuleMode; //special mode when this delay effect is being used in a FeedbackModule
};

#endif /* defined(__modularSynth__DelayEffect__) */


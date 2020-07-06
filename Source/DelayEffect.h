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
   void SetFeedbackModuleMode(bool feedbackMode);
   
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
   Checkbox* mDryCheckbox;
   Checkbox* mAcceptInputCheckbox;
   
   float mWidth;
   float mHeight;
   
   bool mFeedbackModuleMode; //special mode when this delay effect is being used in a FeedbackModule
};

#endif /* defined(__modularSynth__DelayEffect__) */


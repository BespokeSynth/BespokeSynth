//
//  LiveGranulator.h
//  modularSynth
//
//  Created by Ryan Challinor on 10/2/13.
//
//

#ifndef __modularSynth__LiveGranulator__
#define __modularSynth__LiveGranulator__

#include <iostream>
#include "IAudioEffect.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Granulator.h"
#include "Slider.h"
#include "RollingBuffer.h"
#include "Transport.h"
#include "DropdownList.h"

#define FREEZE_EXTRA_SAMPLES_COUNT 2*gSampleRate

class LiveGranulator : public IAudioEffect, public IFloatSliderListener, public ITimeListener, public IDropdownListener
{
public:
   LiveGranulator();
   virtual ~LiveGranulator();
   
   static IAudioEffect* Create() { return new LiveGranulator(); }
   
   string GetTitleLabel() override { return "granulator"; }
   void CreateUIControls() override;
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "granulator"; }
   
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
private:
   void Freeze();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w = mWidth; h = mHeight; }
   bool Enabled() const override { return mEnabled; }   
   
   float mBufferLength;
   RollingBuffer mBuffer;
   Granulator mGranulator;
   FloatSlider* mGranOverlap;
   FloatSlider* mGranSpeed;
   FloatSlider* mGranLengthMs;
   FloatSlider* mGranPosRandomize;
   FloatSlider* mGranSpeedRandomize;
   FloatSlider* mGranSpacingRandomize;
   Checkbox* mGranOctaveCheckbox;
   bool mAdd;
   Checkbox* mAddCheckbox;
   bool mFreeze;
   Checkbox* mFreezeCheckbox;
   int mFreezeExtraSamples;
   float mPos;
   FloatSlider* mPosSlider;
   float mDCEstimate[ChannelBuffer::kMaxNumChannels];   //estimate of DC offset
   NoteInterval mAutoCaptureInterval;
   DropdownList* mAutoCaptureDropdown;
   
   float mWidth;
   float mHeight;
};

#endif /* defined(__modularSynth__LiveGranulator__) */


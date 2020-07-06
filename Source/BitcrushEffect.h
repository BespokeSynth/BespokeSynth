//
//  BitcrushEffect.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#ifndef __additiveSynth__BitcrushEffect__
#define __additiveSynth__BitcrushEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class BitcrushEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   BitcrushEffect();
   
   static IAudioEffect* Create() { return new BitcrushEffect(); }
   
   string GetTitleLabel() override { return "bitcrush"; }
   void CreateUIControls() override;
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "bitcrush"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mCrush;
   float mDownsample;
   int mSampleCounter[ChannelBuffer::kMaxNumChannels];
   float mHeldDownsample[ChannelBuffer::kMaxNumChannels];
   FloatSlider* mCrushSlider;
   FloatSlider* mDownsampleSlider;
   
   float mWidth;
   float mHeight;
};

#endif /* defined(__additiveSynth__BitcrushEffect__) */


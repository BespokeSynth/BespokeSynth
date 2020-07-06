//
//  NoiseEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#ifndef __modularSynth__NoiseEffect__
#define __modularSynth__NoiseEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class NoiseEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   NoiseEffect();
   
   static IAudioEffect* Create() { return new NoiseEffect(); }
   
   string GetTitleLabel() override { return "noisify"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "noisify"; }

   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=60; }
   bool Enabled() const override { return mEnabled; }

   
   float mAmount;
   int mWidth;
   int mSampleCounter;
   float mRandom;
   FloatSlider* mAmountSlider;
   IntSlider* mWidthSlider;
   
};

#endif /* defined(__modularSynth__NoiseEffect__) */


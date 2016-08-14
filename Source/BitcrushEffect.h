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
#include "IAudioProcessor.h"
#include "Slider.h"
#include "Checkbox.h"

class BitcrushEffect : public IAudioProcessor, public IIntSliderListener, public IFloatSliderListener
{
public:
   BitcrushEffect();
   
   static IAudioProcessor* Create() { return new BitcrushEffect(); }
   
   string GetTitleLabel() override { return "bitcrush"; }
   void CreateUIControls() override;
   
   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "bitcrush"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int& y) override;
   bool Enabled() const override { return mEnabled; }
   
   float mCrush;
   int mDownsample;
   int mSampleCounter;
   float mHeldDownsample;
   FloatSlider* mCrushSlider;
   IntSlider* mDownsampleSlider;
};

#endif /* defined(__additiveSynth__BitcrushEffect__) */


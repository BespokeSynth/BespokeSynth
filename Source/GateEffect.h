//
//  GateEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/19/13.
//
//

#ifndef __modularSynth__GateEffect__
#define __modularSynth__GateEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class GateEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener
{
public:
   GateEffect();
   static IAudioEffect* Create() { return new GateEffect(); }
   
   string GetTitleLabel() override { return "gate"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   string GetType() override { return "gate"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=50; }
   bool Enabled() const override { return mEnabled; }
   
   float mThreshold;
   float mAttackTime;
   float mReleaseTime;
   FloatSlider* mThresholdSlider;
   FloatSlider* mAttackSlider;
   FloatSlider* mReleaseSlider;
   float mEnvelope;
   float mPeak;
};

#endif /* defined(__modularSynth__GateEffect__) */


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
#include "IAudioProcessor.h"
#include "Slider.h"
#include "Checkbox.h"

class GateEffect : public IAudioProcessor, public IIntSliderListener, public IFloatSliderListener
{
public:
   GateEffect();
   static IAudioProcessor* Create() { return new GateEffect(); }
   
   string GetTitleLabel() override { return "gate"; }
   void CreateUIControls() override;

   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   string GetType() override { return "gate"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int&y) override { x=120; y=50; }
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


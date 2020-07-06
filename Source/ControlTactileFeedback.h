//
//  ControlTactileFeedback.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/9/14.
//
//

#ifndef __modularSynth__ControlTactileFeedback__
#define __modularSynth__ControlTactileFeedback__

#include <iostream>
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class ControlTactileFeedback : public IAudioSource, public IDrawableModule, public IFloatSliderListener
{
public:
   ControlTactileFeedback();
   ~ControlTactileFeedback();
   static IDrawableModule* Create() { return new ControlTactileFeedback(); }
   
   string GetTitleLabel() override { return "tactile"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   
   void CheckboxUpdated(Checkbox* checkbox) override {}
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override { width=80; height=60; }
   
   float mPhase;
   float mPhaseInc;
   
   
   
   float mVolume;
   FloatSlider* mVolumeSlider;
};


#endif /* defined(__modularSynth__ControlTactileFeedback__) */


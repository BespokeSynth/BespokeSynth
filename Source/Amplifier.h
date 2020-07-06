//
//  Amplifier.h
//  modularSynth
//
//  Created by Ryan Challinor on 7/13/13.
//
//

#ifndef __modularSynth__Amplifier__
#define __modularSynth__Amplifier__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"

class Amplifier : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   Amplifier();
   virtual ~Amplifier();
   static IDrawableModule* Create() { return new Amplifier(); }
   
   string GetTitleLabel() override { return "gain"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=120; h=22; }
   bool Enabled() const override { return mEnabled; }
   
   float mGain;
   FloatSlider* mGainSlider;
};


#endif /* defined(__modularSynth__Amplifier__) */


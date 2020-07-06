//
//  Muter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/26/13.
//
//

#ifndef __modularSynth__Muter__
#define __modularSynth__Muter__

#include <iostream>
#include "IAudioEffect.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Ramp.h"
#include "Slider.h"

class Muter : public IAudioEffect, public IFloatSliderListener
{
public:
   Muter();
   virtual ~Muter();
   
   static IAudioEffect* Create() { return new Muter(); }
   
   string GetTitleLabel() override { return "muter"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override {}
   string GetType() override { return "muter"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

private:
   void Go();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=80; h=38; }
   bool Enabled() const override { return true; }

   bool mPass;

   Checkbox* mPassCheckbox;
   Ramp mRamp;
   float mRampTimeMs;
   FloatSlider* mRampTimeSlider;
};


#endif /* defined(__modularSynth__Muter__) */


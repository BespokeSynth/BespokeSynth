//
//  DebugAudioSource.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/1/14.
//
//

#ifndef __Bespoke__DebugAudioSource__
#define __Bespoke__DebugAudioSource__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"

class DebugAudioSource : public IAudioSource, public IDrawableModule, public IFloatSliderListener
{
public:
   DebugAudioSource();
   ~DebugAudioSource();
   static IDrawableModule* Create() { return new DebugAudioSource(); }
   
   string GetTitleLabel() override { return "debug"; }
   
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
   
   
   
};

#endif /* defined(__Bespoke__DebugAudioSource__) */

/*
  ==============================================================================

    SignalClamp.h
    Created: 1 Dec 2019 3:24:55pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"

class SignalClamp : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   SignalClamp();
   virtual ~SignalClamp();
   static IDrawableModule* Create() { return new SignalClamp(); }
   
   string GetTitleLabel() override { return "signal clamp"; }
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
   void GetModuleDimensions(float& w, float& h) override { w=120; h=40; }
   bool Enabled() const override { return mEnabled; }
   
   float mMin;
   FloatSlider* mMinSlider;
   float mMax;
   FloatSlider* mMaxSlider;
};

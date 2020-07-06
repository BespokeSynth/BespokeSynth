/*
  ==============================================================================

    PulseChance.h
    Created: 4 Feb 2020 12:17:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Slider.h"

class PulseChance : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener
{
public:
   PulseChance();
   virtual ~PulseChance();
   static IDrawableModule* Create() { return new PulseChance(); }
   
   string GetTitleLabel() override { return "pulse chance"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float mChance;
   FloatSlider* mChanceSlider;
   float mLastRejectTime;
   float mLastAcceptTime;
};

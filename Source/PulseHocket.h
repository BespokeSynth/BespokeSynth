/*
  ==============================================================================

    PulseHocket.h
    Created: 22 Feb 2020 10:40:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Slider.h"

class PulseHocket : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener
{
public:
   PulseHocket();
   virtual ~PulseHocket();
   static IDrawableModule* Create() { return new PulseHocket(); }
   
   string GetTitleLabel() override { return "pulse hocket"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return true; }
   
   static const int kMaxDestinations = 5;
   float mWeight[kMaxDestinations];
   FloatSlider* mWeightSlider[kMaxDestinations];
   PatchCableSource* mDestinationCables[kMaxDestinations];
   float mWidth;
   float mHeight;
};

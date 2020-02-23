/*
  ==============================================================================

    PulseGate.h
    Created: 22 Feb 2020 10:39:40pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Slider.h"

class PulseGate : public IDrawableModule, public IPulseSource, public IPulseReceiver
{
public:
   PulseGate();
   virtual ~PulseGate();
   static IDrawableModule* Create() { return new PulseGate(); }
   
   string GetTitleLabel() override { return "pulse gate"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IPulseReceiver
   void OnPulse(float velocity, int samplesTo, int flags) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return true; }
   
   bool mAllow;
   Checkbox* mAllowCheckbox;
   float mWidth;
   float mHeight;
};

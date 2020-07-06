/*
  ==============================================================================

    Inverter.h
    Created: 13 Nov 2019 10:16:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"

class Inverter : public IAudioProcessor, public IDrawableModule
{
public:
   Inverter();
   virtual ~Inverter();
   static IDrawableModule* Create() { return new Inverter(); }
   
   string GetTitleLabel() override { return "inverter"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=120; h=12; }
   bool Enabled() const override { return mEnabled; }
};

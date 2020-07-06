/*
  ==============================================================================

    DCOffset.h
    Created: 1 Dec 2019 3:24:31pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"

class DCOffset : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   DCOffset();
   virtual ~DCOffset();
   static IDrawableModule* Create() { return new DCOffset(); }
   
   string GetTitleLabel() override { return "dc offset"; }
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
   
   float mOffset;
   FloatSlider* mOffsetSlider;
};

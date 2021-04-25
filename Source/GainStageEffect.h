/*
  ==============================================================================

    GainStage.h
    Created: 24 Apr 2021 3:47:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class GainStageEffect : public IAudioEffect, public IFloatSliderListener
{
public:
   GainStageEffect();
   static IAudioEffect* Create() { return new GainStageEffect(); }
   
   string GetTitleLabel() override { return "gain stage"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   string GetType() override { return "gainstage"; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=20; }
   bool Enabled() const override { return mEnabled; }
   
   float mGain;
   FloatSlider* mGainSlider;
};

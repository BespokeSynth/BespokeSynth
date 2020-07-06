/*
  ==============================================================================

    AudioSend.h
    Created: 22 Oct 2017 1:23:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "PatchCableSource.h"
#include "Slider.h"

class AudioSend : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   AudioSend();
   virtual ~AudioSend();
   static IDrawableModule* Create() { return new AudioSend(); }
   
   string GetTitleLabel() override { return "send"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return 2; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=86; h=38; }
   bool Enabled() const override { return mEnabled; }
   
   bool mCrossfade;
   Checkbox* mCrossfadeCheckbox;
   float mAmount;
   FloatSlider* mAmountSlider;
   RollingBuffer mVizBuffer2;
   PatchCableSource* mPatchCableSource2;
};

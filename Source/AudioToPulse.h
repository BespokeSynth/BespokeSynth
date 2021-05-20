/*
  ==============================================================================

    AudioToPulse.h
    Created: 31 Mar 2021 10:18:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IPulseReceiver.h"
#include "Slider.h"

class PatchCableSource;

class AudioToPulse : public IDrawableModule, public IPulseSource, public IAudioProcessor, public IFloatSliderListener
{
public:
   AudioToPulse();
   virtual ~AudioToPulse();
   static IDrawableModule* Create() { return new AudioToPulse(); }

   string GetTitleLabel() override { return "audio to pulse"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void Process(double time) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;

   FloatSlider* mThresholdSlider;
   FloatSlider* mReleaseSlider;
   float mVal;
   float mThreshold;
   float mRelease;
   float mReleaseFactor;
};
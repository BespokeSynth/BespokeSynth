/*
  ==============================================================================

    AudioToCV.h
    Created: 18 Nov 2017 10:46:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class AudioToCV : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   AudioToCV();
   virtual ~AudioToCV();
   static IDrawableModule* Create() { return new AudioToCV(); }
   
   string GetTitleLabel() override { return "audio to cv"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   InputMode GetInputMode() override { return kInputMode_Mono; }
   void Process(double time) override;
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=106; h=17*3+2; }
   bool Enabled() const override { return mEnabled; }
   
   float mGain;
   float* mModulationBuffer;
   FloatSlider* mGainSlider;
};


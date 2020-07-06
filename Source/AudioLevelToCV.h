/*
  ==============================================================================

    AudioLevelToCV.h
    Created: 9 Oct 2018 10:26:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class AudioLevelToCV : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   AudioLevelToCV();
   virtual ~AudioLevelToCV();
   static IDrawableModule* Create() { return new AudioLevelToCV(); }
   
   string GetTitleLabel() override { return "level to cv"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   InputMode GetInputMode() override { return kInputMode_Mono; }
   void Process(double time) override;
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=106; h=17*5+2; }
   bool Enabled() const override { return mEnabled; }
   
   float mGain;
   float* mModulationBuffer;
   FloatSlider* mGainSlider;
   FloatSlider* mAttackSlider;
   FloatSlider* mReleaseSlider;
   float mVal;
   float mAttack;
   float mRelease;
   float mAttackFactor;
   float mReleaseFactor;
};



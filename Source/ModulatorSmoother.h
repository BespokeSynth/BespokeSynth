/*
  ==============================================================================

    ModulatorSmoother.h
    Created: 29 Nov 2017 9:35:31pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"
#include "Transport.h"
#include "Ramp.h"

class PatchCableSource;

class ModulatorSmoother : public IDrawableModule, public IFloatSliderListener, public IModulator, public IAudioPoller
{
public:
   ModulatorSmoother();
   virtual ~ModulatorSmoother();
   static IDrawableModule* Create() { return new ModulatorSmoother(); }
   
   string GetTitleLabel() override { return "smoother"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   FloatSlider* GetTarget() { return mTarget; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=106; h=17*2+4; }
   bool Enabled() const override { return mEnabled; }
   
   float mInput;
   float mSmooth;
   Ramp mRamp;
   
   FloatSlider* mInputSlider;
   FloatSlider* mSmoothSlider;
};

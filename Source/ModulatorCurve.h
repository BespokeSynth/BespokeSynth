/*
  ==============================================================================

    ModulatorCurve.h
    Created: 29 Nov 2017 8:56:47pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorCurve : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorCurve();
   virtual ~ModulatorCurve();
   static IDrawableModule* Create() { return new ModulatorCurve(); }
   
   string GetTitleLabel() override { return "curve"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   FloatSlider* GetTarget() { return mTarget; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=106; h=17*2+4; }
   bool Enabled() const override { return mEnabled; }
   
   float mInput;
   float mCurve;
   
   FloatSlider* mInputSlider;
   FloatSlider* mCurveSlider;
};

/*
  ==============================================================================

    ModulatorMult.h
    Created: 29 Nov 2017 8:56:31pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorMult : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorMult();
   virtual ~ModulatorMult();
   static IDrawableModule* Create() { return new ModulatorMult(); }
   
   string GetTitleLabel() override { return "mult"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
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
   
   float mValue1;
   float mValue2;
   
   FloatSlider* mValue1Slider;
   FloatSlider* mValue2Slider;
};

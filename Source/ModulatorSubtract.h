/*
  ==============================================================================

    ModulatorSubtract.h
    Created: 9 Dec 2019 10:11:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorSubtract : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorSubtract();
   virtual ~ModulatorSubtract();
   static IDrawableModule* Create() { return new ModulatorSubtract(); }
   
   string GetTitleLabel() override { return "subtract"; }
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

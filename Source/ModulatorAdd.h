/*
  ==============================================================================

    ModulatorAdd.h
    Created: 19 Nov 2017 2:04:23pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorAdd : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorAdd();
   virtual ~ModulatorAdd();
   static IDrawableModule* Create() { return new ModulatorAdd(); }
   
   string GetTitleLabel() override { return "add"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   float& GetMin() override { return mMin; }
   float& GetMax() override { return mMax; }
   
   FloatSlider* GetTarget() { return mTarget; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=106; h=17*3+4; }
   bool Enabled() const override { return mEnabled; }
   
   float mValue1;
   float mValue2;
   float mValue2Range;
   float mMin;
   float mMax;
   PatchCableSource* mTargetCable;
   FloatSlider* mTarget;
   
   FloatSlider* mValue1Slider;
   FloatSlider* mValue2Slider;
   FloatSlider* mValue2RangeSlider;
};


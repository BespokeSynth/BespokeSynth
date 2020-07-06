/*
  ==============================================================================

    ModulatorAddCentered.h
    Created: 22 Nov 2017 9:50:16am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorAddCentered : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorAddCentered();
   virtual ~ModulatorAddCentered();
   static IDrawableModule* Create() { return new ModulatorAddCentered(); }
   
   string GetTitleLabel() override { return "add centered"; }
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
   void GetModuleDimensions(float& w, float& h) override { w=106; h=17*3+4; }
   bool Enabled() const override { return mEnabled; }
   
   float mValue1;
   float mValue2;
   float mValue2Range;
   
   FloatSlider* mValue1Slider;
   FloatSlider* mValue2Slider;
   FloatSlider* mValue2RangeSlider;
};

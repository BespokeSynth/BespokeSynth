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
#include "EnvelopeEditor.h"

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
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   FloatSlider* GetTarget() { return mTarget; }
   
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=106; h=121; }
   bool Enabled() const override { return mEnabled; }
   
   void OnClicked(int x, int y, bool right) override;
   
   float mInput;
   EnvelopeControl mEnvelopeControl;
   ::ADSR mAdsr;
   
   FloatSlider* mInputSlider;
};

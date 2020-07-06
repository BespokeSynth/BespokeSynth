/*
  ==============================================================================

    ModulatorGravity.h
    Created: 30 Apr 2020 3:56:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"
#include "Transport.h"
#include "Ramp.h"
#include "ClickButton.h"

class PatchCableSource;

class ModulatorGravity : public IDrawableModule, public IFloatSliderListener, public IModulator, public IAudioPoller, public IButtonListener
{
public:
   ModulatorGravity();
   virtual ~ModulatorGravity();
   static IDrawableModule* Create() { return new ModulatorGravity(); }
   
   string GetTitleLabel() override { return "gravity"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   FloatSlider* GetTarget() { return mTarget; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   float mValue;
   float mVelocity;
   Ramp mRamp;
   float mGravity;
   float mKickAmount;
   float mDrag;
   
   FloatSlider* mGravitySlider;
   FloatSlider* mKickAmountSlider;
   FloatSlider* mDragSlider;
   ClickButton* mKickButton;
};

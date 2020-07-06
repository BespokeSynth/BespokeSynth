/*
  ==============================================================================

    VelocityToCV.h
    Created: 28 Nov 2017 9:43:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IModulator.h"
#include "Slider.h"

class PatchCableSource;

class VelocityToCV : public IDrawableModule, public INoteReceiver, public IModulator, public IFloatSliderListener
{
public:
   VelocityToCV();
   virtual ~VelocityToCV();
   static IDrawableModule* Create() { return new VelocityToCV(); }
   
   string GetTitleLabel() override { return "velocity to cv"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IModulator
   virtual float Value(int samplesIn = 0) override;
   virtual bool Active() const override { return mEnabled; }
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 106; height=17*2+2; }
   bool Enabled() const override { return mEnabled; }
   
   int mVelocity;
};

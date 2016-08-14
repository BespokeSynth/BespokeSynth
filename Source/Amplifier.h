//
//  Amplifier.h
//  modularSynth
//
//  Created by Ryan Challinor on 7/13/13.
//
//

#ifndef __modularSynth__Amplifier__
#define __modularSynth__Amplifier__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"

class Amplifier : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener
{
public:
   Amplifier();
   virtual ~Amplifier();
   static IDrawableModule* Create() { return new Amplifier(); }
   
   string GetTitleLabel() override { return "amplifier"; }
   void CreateUIControls() override;
   
   void SetBoost(float boost) { mBoost = boost; }
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=120; h=22; }
   bool Enabled() const override { return mEnabled; }
   
   
   int mInputBufferSize;
   float* mInputBuffer;
   float mBoost;
   FloatSlider* mBoostSlider;
};


#endif /* defined(__modularSynth__Amplifier__) */


//
//  FreqDelay.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/10/13.
//
//

#ifndef __modularSynth__FreqDelay__
#define __modularSynth__FreqDelay__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "DelayEffect.h"
#include "Slider.h"

class FreqDelay : public IAudioProcessor, public IDrawableModule, public INoteReceiver, public IFloatSliderListener
{
public:
   FreqDelay();
   virtual ~FreqDelay();
   static IDrawableModule* Create() { return new FreqDelay(); }
   
   string GetTitleLabel() override { return "freq delay"; }
   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 130; height = 110; }
   bool Enabled() const override { return true; }

   ChannelBuffer mDryBuffer;
   float mDryWet;
   FloatSlider* mDryWetSlider;

   DelayEffect mDelayEffect;
};

#endif /* defined(__modularSynth__FreqDelay__) */


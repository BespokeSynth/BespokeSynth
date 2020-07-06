//
//  RingModulator.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#ifndef __modularSynth__RingModulator__
#define __modularSynth__RingModulator__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "INoteReceiver.h"

class RingModulator : public IAudioProcessor, public IDrawableModule, public IButtonListener, public IFloatSliderListener, public INoteReceiver
{
public:
   RingModulator();
   virtual ~RingModulator();
   static IDrawableModule* Create() { return new RingModulator(); }
   
   string GetTitleLabel() override { return "ring modulator"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 130; height = 68; }
   bool Enabled() const override { return mEnabled; }

   ChannelBuffer mDryBuffer;

   float mFreq;
   float mDryWet;
   float mVolume;
   FloatSlider* mFreqSlider;
   FloatSlider* mDryWetSlider;
   FloatSlider* mVolumeSlider;
   
   EnvOscillator mModOsc;
   float mPhase;
   Ramp mFreqRamp;
   float mGlideTime;
   FloatSlider* mGlideSlider;
};


#endif /* defined(__modularSynth__RingModulator__) */


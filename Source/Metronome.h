//
//  Metronome.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#ifndef __modularSynth__Metronome__
#define __modularSynth__Metronome__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"

class Metronome : public IAudioSource, public ITimeListener, public IDrawableModule, public IFloatSliderListener
{
public:
   Metronome();
   ~Metronome();
   static IDrawableModule* Create() { return new Metronome(); }
   
   string GetTitleLabel() override { return "metronome"; }
   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override {}

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override { width=80; height=35; }

   float mPhase;
   float mPhaseInc;

   
   EnvOscillator mOsc;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
};

#endif /* defined(__modularSynth__Metronome__) */


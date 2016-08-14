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
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "INoteReceiver.h"

class RingModulator : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IButtonListener, public IFloatSliderListener, public INoteReceiver
{
public:
   RingModulator();
   virtual ~RingModulator();
   static IDrawableModule* Create() { return new RingModulator(); }
   
   string GetTitleLabel() override { return "ring modulator"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;

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
   void GetModuleDimensions(int& x, int& y) override { x = 130; y = 52; }
   bool Enabled() const override { return mEnabled; }

   int mInputBufferSize;
   float* mInputBuffer;
   float* mDryBuffer;

   float mDryWet;
   float mVolume;
   FloatSlider* mDryWetSlider;
   FloatSlider* mVolumeSlider;
   
   
   EnvOscillator mModOsc;
   float mPhase;
   Ramp mFreq;
   float mGlideTime;
   FloatSlider* mGlideSlider;
};


#endif /* defined(__modularSynth__RingModulator__) */


//
//  PitchChorus.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#ifndef __Bespoke__PitchChorus__
#define __Bespoke__PitchChorus__

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "PitchShifter.h"
#include "INoteReceiver.h"
#include "Ramp.h"
#include "Checkbox.h"

class PitchChorus : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public INoteReceiver
{
public:
   PitchChorus();
   virtual ~PitchChorus();
   static IDrawableModule* Create() { return new PitchChorus(); }
   
   string GetTitleLabel() override { return "pitchchorus"; }
   void CreateUIControls() override;
   
   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void CheckboxUpdated(Checkbox* checkbox) override {}
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=120; h=22; }
   bool Enabled() const override { return mEnabled; }
   
   static const int kNumShifters = 5;
   
   struct PitchShifterVoice
   {
      PitchShifterVoice() : mShifter(1024), mOn(false), mPitch(-1) {}
      PitchShifter mShifter;
      bool mOn;
      Ramp mRamp;
      int mPitch;
   };
   
   float* mOutputBuffer;
   PitchShifterVoice mShifters[kNumShifters];
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
};


#endif /* defined(__Bespoke__PitchChorus__) */

//
//  PitchChorus.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#ifndef __Bespoke__PitchChorus__
#define __Bespoke__PitchChorus__

#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "PitchShifter.h"
#include "INoteReceiver.h"
#include "Ramp.h"
#include "Checkbox.h"

class PitchChorus : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener, public INoteReceiver
{
public:
   PitchChorus();
   virtual ~PitchChorus();
   static IDrawableModule* Create() { return new PitchChorus(); }
   
   string GetTitleLabel() override { return "pitchchorus"; }
   void CreateUIControls() override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void CheckboxUpdated(Checkbox* checkbox) override {}
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=120; h=22; }
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
   
   int mInputBufferSize;
   float* mInputBuffer;
   float* mOutputBuffer;
   PitchShifterVoice mShifters[kNumShifters];
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
};


#endif /* defined(__Bespoke__PitchChorus__) */

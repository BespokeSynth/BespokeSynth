/*
  ==============================================================================

    PulseDelayer.h
    Created: 15 Feb 2020 2:53:22pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"

class PulseDelayer : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener, public IAudioPoller
{
public:
   PulseDelayer();
   ~PulseDelayer();
   static IDrawableModule* Create() { return new PulseDelayer(); }
   
   string GetTitleLabel() override { return "pulse delayer"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;
   
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   struct PulseInfo
   {
      float mVelocity;
      int mFlags;
      double mTriggerTime;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   float mDelay;
   FloatSlider* mDelaySlider;
   
   float mLastPulseTime;
   
   static const int kQueueSize = 50;
   PulseInfo mInputPulses[kQueueSize];
   int mConsumeIndex;
   int mAppendIndex;
};

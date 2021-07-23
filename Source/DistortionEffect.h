//
//  DistortionEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__DistortionEffect__
#define __modularSynth__DistortionEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "BiquadFilter.h"
#include "PeakTracker.h"

class DistortionEffect : public IAudioEffect, public IFloatSliderListener, public IDropdownListener
{
public:
   DistortionEffect();
   
   static IAudioEffect* Create() { return new DistortionEffect(); }
   
   string GetTitleLabel() override { return "distort"; }
   void CreateUIControls() override;
   
   void SetClip(float amount);
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "distortion"; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
private:
   enum DistortionType
   {
      kClean,
      kWarm,
      kDirty,
      kSoft,
      kAsymmetric,
      kFold,
      kGrungy
   };
   
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;
   
   DistortionType mType;
   float mClip;
   float mGain;
   float mPreamp;
   float mFuzzAmount;
   bool mRemoveInputDC;
   
   DropdownList* mTypeDropdown;
   FloatSlider* mClipSlider;
   FloatSlider* mPreampSlider;
   Checkbox* mRemoveInputDCCheckbox;
   FloatSlider* mFuzzAmountSlider;
   BiquadFilter mDCRemover[ChannelBuffer::kMaxNumChannels];
   PeakTracker mPeakTracker[ChannelBuffer::kMaxNumChannels];
};

#endif /* defined(__modularSynth__DistortionEffect__) */


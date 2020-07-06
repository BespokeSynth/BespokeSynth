//
//  BiquadFilterEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/29/12.
//
//

#ifndef __modularSynth__BiquadFilterEffect__
#define __modularSynth__BiquadFilterEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "BiquadFilter.h"
#include "RadioButton.h"

class BiquadFilterEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   BiquadFilterEffect();
   ~BiquadFilterEffect();
   
   static IAudioEffect* Create() { return new BiquadFilterEffect(); }
   
   string GetTitleLabel() override { return "biquad"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetFilterType(FilterType type) { mBiquad[0].SetFilterType(type); }
   void SetFilterParams(float f, float q) { mBiquad[0].SetFilterParams(f, q); }
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "biquad"; }
   
   bool MouseMoved(float x, float y) override;

   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;
   
private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   void ResetFilter();
   
   RadioButton* mTypeSelector;
   
   FloatSlider* mFSlider;
   FloatSlider* mQSlider;
   FloatSlider* mGSlider;
   bool mMouseControl;
   
   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels];
   ChannelBuffer mDryBuffer;
   
   bool mCoefficientsHaveChanged;
};

#endif /* defined(__modularSynth__BiquadFilterEffect__) */



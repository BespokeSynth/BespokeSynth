//
//  TremoloEffect.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#ifndef __modularSynth__TremoloEffect__
#define __modularSynth__TremoloEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"
#include "LFO.h"
#include "DropdownList.h"

class TremoloEffect : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   TremoloEffect();
   
   static IAudioEffect* Create() { return new TremoloEffect(); }
   
   string GetTitleLabel() override { return "tremolo"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "tremolo"; }

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }

   float mAmount;
   FloatSlider* mAmountSlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   
   LFO mLFO;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   OscillatorType mOscType;
   DropdownList* mOscSelector;
   FloatSlider* mDutySlider;
   float mDuty;
   static const int kAntiPopWindowSize = 300;
   float mWindow[kAntiPopWindowSize];
   int mWindowPos;
   float mWidth;
   float mHeight;
};


#endif /* defined(__modularSynth__TremoloEffect__) */


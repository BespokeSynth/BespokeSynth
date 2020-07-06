//
//  Pumper.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#ifndef __modularSynth__Pumper__
#define __modularSynth__Pumper__

#include <iostream>
#include "IAudioEffect.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "LFO.h"

class Pumper : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   Pumper();
   virtual ~Pumper();
   
   static IAudioEffect* Create() { return new Pumper(); }
   
   string GetTitleLabel() override { return "pumper"; }
   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "pumper"; }

   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w = mWidth; h = mHeight; }
   bool Enabled() const override { return mEnabled; }
   double GetIntervalPos(double time);
   void SyncToAdsr();
   
   FloatSlider* mAmountSlider;
   FloatSlider* mLengthSlider;
   FloatSlider* mCurveSlider;
   FloatSlider* mAttackSlider;
   
   ::ADSR mAdsr;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   float mLastValue;
   float mAmount;
   float mLength;
   float mAttack;
   
   float mWidth;
   float mHeight;
};

#endif /* defined(__modularSynth__Pumper__) */


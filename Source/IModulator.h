/*
  ==============================================================================

    IModulator.h
    Created: 16 Nov 2017 9:59:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "Slider.h"
#include "IPollable.h"

class PatchCableSource;

class IModulator : public IPollable
{
public:
   IModulator();
   virtual ~IModulator() {}
   virtual float Value(int samplesIn = 0) = 0;
   virtual bool Active() const = 0;
   virtual bool CanAdjustRange() const { return true; }
   virtual bool InitializeWithZeroRange() const { return false; }
   float& GetMin() { return mTarget ? mTarget->GetModulatorMin() : mDummyMin; }
   float& GetMax() { return mTarget ? mTarget->GetModulatorMax() : mDummyMax; }
   void OnModulatorRepatch();
   void Poll() override;
protected:
   void InitializeRange();
   
   float mDummyMin;
   float mDummyMax;
   
   PatchCableSource* mTargetCable;
   FloatSlider* mMinSlider;
   FloatSlider* mMaxSlider;
   FloatSlider* mTarget;
   IUIControl* mUIControlTarget;
};

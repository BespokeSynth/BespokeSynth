/*
  ==============================================================================

    IModulator.h
    Created: 16 Nov 2017 9:59:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "Slider.h"

class PatchCableSource;

class IModulator
{
public:
   IModulator();
   virtual ~IModulator() {}
   virtual float Value(int samplesIn = 0) = 0;
   virtual bool Active() const = 0;
   virtual bool CanAdjustRange() const { return true; }
   virtual bool InitializeWithZeroRange() const { return false; }
   float& GetMin() { return mTarget->GetModulatorMin(); }
   float& GetMax() { return mTarget->GetModulatorMax(); }
   void OnModulatorRepatch();
protected:
   void InitializeRange();
   
   float mDummyMin;
   float mDummyMax;
   
   PatchCableSource* mTargetCable;
   FloatSlider* mMinSlider;
   FloatSlider* mMaxSlider;
   FloatSlider* mTarget;
};

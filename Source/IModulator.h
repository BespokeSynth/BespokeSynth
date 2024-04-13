/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
   virtual ~IModulator();
   virtual float Value(int samplesIn = 0) = 0;
   virtual bool Active() const = 0;
   virtual bool CanAdjustRange() const { return true; }
   virtual bool InitializeWithZeroRange() const { return false; }
   float& GetMin() { return mTargets[0].mSliderTarget ? mTargets[0].mSliderTarget->GetModulatorMin() : mDummyMin; }
   float& GetMax() { return mTargets[0].mSliderTarget ? mTargets[0].mSliderTarget->GetModulatorMax() : mDummyMax; }
   void OnModulatorRepatch();
   void Poll() override;
   float GetRecentChange() const;
   void OnRemovedFrom(IUIControl* control);
   int GetNumTargets() const;

protected:
   void InitializeRange(float currentValue, float min, float max, FloatSlider::Mode sliderMode);

   FloatSlider* GetSliderTarget() const { return mTargets[0].mSliderTarget; }

   float mDummyMin{ 0 };
   float mDummyMax{ 1 };

   struct Target
   {
      FloatSlider* mSliderTarget{ nullptr };
      IUIControl* mUIControlTarget{ nullptr };

      bool RequiresManualPolling() { return mUIControlTarget != nullptr && mSliderTarget == nullptr; }
   };

   PatchCableSource* mTargetCable{ nullptr };
   FloatSlider* mMinSlider{ nullptr };
   FloatSlider* mMaxSlider{ nullptr };
   std::array<Target, 10> mTargets;
   float mLastPollValue{ 0 };
   float mSmoothedValue{ 0 };
};

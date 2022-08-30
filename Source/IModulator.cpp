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

    IModulator.cpp
    Created: 16 Nov 2017 9:59:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IModulator.h"
#include "Slider.h"
#include "PatchCableSource.h"
#include "ModularSynth.h"

IModulator::IModulator()
: mDummyMin(0)
, mDummyMax(1)
, mTargetCable(nullptr)
, mMinSlider(nullptr)
, mMaxSlider(nullptr)
, mSliderTarget(nullptr)
, mUIControlTarget(nullptr)
, mLastPollValue(0)
, mSmoothedValue(0)
{
}

IModulator::~IModulator()
{
   TheSynth->RemoveExtraPoller(this);
}

void IModulator::OnModulatorRepatch()
{
   if (mTargetCable && mTargetCable->GetPatchCables().empty() == false)
   {
      IUIControl* newTarget = dynamic_cast<IUIControl*>(mTargetCable->GetPatchCables()[0]->GetTarget());
      if (newTarget != mUIControlTarget)
      {
         if (mSliderTarget != nullptr && mSliderTarget->GetModulator() == this)
            mSliderTarget->SetModulator(nullptr); //clear old target's pointer to this

         mUIControlTarget = newTarget;
         mSliderTarget = dynamic_cast<FloatSlider*>(mUIControlTarget);

         if (mSliderTarget != nullptr)
         {
            mSliderTarget->SetModulator(this);
            InitializeRange();
         }
      }
   }
   else
   {
      if (mSliderTarget != nullptr && mSliderTarget->GetModulator() == this)
         mSliderTarget->SetModulator(nullptr); //clear old target's pointer to this

      mUIControlTarget = nullptr;
      mSliderTarget = nullptr;

      if (mMinSlider)
         mMinSlider->SetVar(&mDummyMin);
      if (mMaxSlider)
         mMaxSlider->SetVar(&mDummyMax);
   }

   TheSynth->RemoveExtraPoller(this);
   //if (RequiresManualPolling())
   TheSynth->AddExtraPoller(this);
}

void IModulator::Poll()
{
   if (Active())
   {
      mLastPollValue = Value();
      const float kBlendRate = -9.65784f;
      float blend = exp2(kBlendRate / ofGetFrameRate()); //framerate-independent blend
      mSmoothedValue = mSmoothedValue * blend + mLastPollValue * (1 - blend);
      if (RequiresManualPolling())
         mUIControlTarget->SetValue(mLastPollValue);  
   }
}

float IModulator::GetRecentChange() const
{
   return mLastPollValue - mSmoothedValue;
}

void IModulator::OnRemovedFrom(IUIControl* control)
{
   if (mTargetCable && mTargetCable->GetTarget() == mUIControlTarget)
      mTargetCable->Clear();
   OnModulatorRepatch();
}

void IModulator::InitializeRange()
{
   if (mSliderTarget != nullptr)
   {
      if (!TheSynth->IsLoadingState())
      {
         if (!TheSynth->IsLoadingModule())
         {
            if (InitializeWithZeroRange())
            {
               GetMin() = mSliderTarget->GetValue();
               GetMax() = mSliderTarget->GetValue();
            }
            else
            {
               GetMin() = mSliderTarget->GetMin();
               GetMax() = mSliderTarget->GetMax();
            }
         }
      }

      if (mMinSlider)
      {
         mMinSlider->SetExtents(mSliderTarget->GetMin(), mSliderTarget->GetMax());
         mMinSlider->SetMode(mSliderTarget->GetMode());
         mMinSlider->SetVar(&GetMin());
      }
      if (mMaxSlider)
      {
         mMaxSlider->SetExtents(mSliderTarget->GetMin(), mSliderTarget->GetMax());
         mMaxSlider->SetMode(mSliderTarget->GetMode());
         mMaxSlider->SetVar(&GetMax());
      }
   }
}

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
{
}

IModulator::~IModulator()
{
   TheSynth->RemoveExtraPoller(this);
}

void IModulator::OnModulatorRepatch()
{
   bool wasEmpty = (mTargets[0].mUIControlTarget == nullptr);

   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      IUIControl* newTarget = nullptr;
      if (mTargetCable != nullptr && i < mTargetCable->GetPatchCables().size())
         newTarget = dynamic_cast<IUIControl*>(mTargetCable->GetPatchCables()[i]->GetTarget());
      if (newTarget != mTargets[i].mUIControlTarget)
      {
         if (mTargets[i].mSliderTarget != nullptr && mTargets[i].mSliderTarget->GetModulator() == this)
            mTargets[i].mSliderTarget->SetModulator(nullptr); //clear old target's pointer to this

         if (i + 1 < mTargets.size() && newTarget == mTargets[i + 1].mUIControlTarget) //one got deleted, shift the rest down
         {
            for (; i < mTargets.size(); ++i)
            {
               if (i + 1 < mTargets.size())
               {
                  mTargets[i].mUIControlTarget = mTargets[i + 1].mUIControlTarget;
                  mTargets[i].mSliderTarget = mTargets[i + 1].mSliderTarget;
               }
               else
               {
                  mTargets[i].mUIControlTarget = nullptr;
                  mTargets[i].mSliderTarget = nullptr;
               }
            }
            break;
         }

         mTargets[i].mUIControlTarget = newTarget;
         mTargets[i].mSliderTarget = dynamic_cast<FloatSlider*>(mTargets[i].mUIControlTarget);

         if (newTarget != nullptr)
         {
            if (mTargets[i].mSliderTarget != nullptr)
            {
               mTargets[i].mSliderTarget->SetModulator(this);
               if (wasEmpty)
                  InitializeRange(mTargets[i].mSliderTarget->GetValue(), mTargets[i].mUIControlTarget->GetModulationRangeMin(), mTargets[i].mUIControlTarget->GetModulationRangeMax(), mTargets[i].mSliderTarget->GetMode());
            }
            else
            {
               if (wasEmpty)
                  InitializeRange(mTargets[i].mUIControlTarget->GetValue(), mTargets[i].mUIControlTarget->GetModulationRangeMin(), mTargets[i].mUIControlTarget->GetModulationRangeMax(), FloatSlider::kNormal);
            }
         }
         else
         {
            if (i == 0)
            {
               if (mMinSlider)
                  mMinSlider->SetVar(&mDummyMin);
               if (mMaxSlider)
                  mMaxSlider->SetVar(&mDummyMax);
            }
         }
      }
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
      for (int i = 0; i < (int)mTargets.size(); ++i)
      {
         if (mTargets[i].RequiresManualPolling())
         {
            if (mTargets[i].mUIControlTarget->ModulatorUsesLiteralValue())
               mTargets[i].mUIControlTarget->SetValue(mLastPollValue, NextBufferTime(false));
            else
               mTargets[i].mUIControlTarget->SetFromMidiCC(mLastPollValue, NextBufferTime(false), true);
         }
      }
   }
}

float IModulator::GetRecentChange() const
{
   return mLastPollValue - mSmoothedValue;
}

void IModulator::OnRemovedFrom(IUIControl* control)
{
   if (mTargetCable)
   {
      auto& cables = mTargetCable->GetPatchCables();
      for (size_t i = 0; i < mTargets.size(); ++i)
      {
         for (auto& cable : cables)
         {
            if (cable->GetTarget() == control && cable->GetTarget() == mTargets[i].mUIControlTarget)
            {
               mTargetCable->RemovePatchCable(cable);
               break;
            }
         }
      }
   }
   OnModulatorRepatch();
}

void IModulator::InitializeRange(float currentValue, float min, float max, FloatSlider::Mode sliderMode)
{
   if (!TheSynth->IsLoadingState())
   {
      if (!TheSynth->IsLoadingModule())
      {
         if (InitializeWithZeroRange())
         {
            GetMin() = currentValue;
            GetMax() = currentValue;
         }
         else
         {
            GetMin() = min;
            GetMax() = max;
         }

         if (mMinSlider)
         {
            mMinSlider->SetExtents(min, max);
            mMinSlider->SetMode(sliderMode);
         }

         if (mMaxSlider)
         {
            mMaxSlider->SetExtents(min, max);
            mMaxSlider->SetMode(sliderMode);
         }
      }
   }

   if (mMinSlider)
      mMinSlider->SetVar(&GetMin());
   if (mMaxSlider)
      mMaxSlider->SetVar(&GetMax());
}

int IModulator::GetNumTargets() const
{
   int ret = 0;
   for (int i = 0; i < (int)mTargets.size(); ++i)
   {
      if (mTargets[i].mUIControlTarget != nullptr)
         ++ret;
   }
   return ret;
}

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
, mTarget(nullptr)
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
   assert(mTargetCable != nullptr);
   
   if (mTargetCable->GetPatchCables().empty() == false)
   {
      IUIControl* newTarget = dynamic_cast<IUIControl*>(mTargetCable->GetPatchCables()[0]->GetTarget());
      if (newTarget != mUIControlTarget)
      {
         if (mTarget != nullptr)
            mTarget->SetModulator(nullptr);  //clear old target's pointer to this
         mUIControlTarget = newTarget;
         mTarget = dynamic_cast<FloatSlider*>(mUIControlTarget);
         if (mTarget != nullptr)
         {
            mTarget->SetModulator(this);
            InitializeRange();
         }
      }
   }
   else
   {
      if (mTarget != nullptr)
         mTarget->SetModulator(nullptr);  //clear old target's pointer to this
      mTarget = nullptr;
      mUIControlTarget = nullptr;
   }
   
   TheSynth->RemoveExtraPoller(this);
   //if (RequiresManualPolling())
      TheSynth->AddExtraPoller(this);
}

void IModulator::Poll()
{
   mLastPollValue = Value();
   const float kBlendRate = -9.65784f;
   float blend = exp2(kBlendRate / ofGetFrameRate()); //framerate-independent blend
   mSmoothedValue = mSmoothedValue * blend + mLastPollValue * (1-blend);
   if (RequiresManualPolling())
      mUIControlTarget->SetFromMidiCC(mLastPollValue, true);
}

float IModulator::GetRecentChange() const
{
   return mLastPollValue - mSmoothedValue;
}

void IModulator::InitializeRange()
{
   if (mTarget != nullptr)
   {
      if (!TheSynth->IsLoadingState())
      {
         if (!TheSynth->IsLoadingModule())
         {
            if (InitializeWithZeroRange())
            {
               GetMin() = mTarget->GetValue();
               GetMax() = mTarget->GetValue();
            }
            else
            {
               GetMin() = mTarget->GetMin();
               GetMax() = mTarget->GetMax();
            }
         }
         
         if (mMinSlider)
         {
            mMinSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
            mMinSlider->SetMode(mTarget->GetMode());
            mMinSlider->SetVar(&GetMin());
         }
         if (mMaxSlider)
         {
            mMaxSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
            mMaxSlider->SetMode(mTarget->GetMode());
            mMaxSlider->SetVar(&GetMax());
         }
      }
   }
}

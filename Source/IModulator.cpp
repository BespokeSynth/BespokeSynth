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
: mMin(0)
, mMax(1)
, mTargetCable(nullptr)
, mMinSlider(nullptr)
, mMaxSlider(nullptr)
, mTarget(nullptr)
{
}

void IModulator::OnModulatorRepatch()
{
   assert(mTargetCable != nullptr);
   
   if (mTargetCable->GetPatchCables().empty() == false)
   {
      FloatSlider* newTarget = dynamic_cast<FloatSlider*>(mTargetCable->GetPatchCables()[0]->GetTarget());
      if (newTarget != mTarget)
      {
         if (mTarget != nullptr)
            mTarget->SetModulator(nullptr);  //clear old target's pointer to this
         mTarget = newTarget;
         mTarget->SetModulator(this);
         InitializeRange();
      }
   }
   else
   {
      if (mTarget != nullptr)
         mTarget->SetModulator(nullptr);  //clear old target's pointer to this
      mTarget = nullptr;
   }
}

void IModulator::InitializeRange()
{
   assert(mTarget != nullptr);
   
   if (!TheSynth->IsLoadingModule())
   {
      if (InitializeWithZeroRange())
      {
         mMin = mTarget->GetValue();
         mMax = mTarget->GetValue();
      }
      else
      {
         mMin = mTarget->GetMin();
         mMax = mTarget->GetMax();
      }
   }
   
   if (mMinSlider)
   {
      mMinSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mMinSlider->SetMode(mTarget->GetMode());
   }
   if (mMaxSlider)
   {
      mMaxSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mMaxSlider->SetMode(mTarget->GetMode());
   }
}

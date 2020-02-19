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
{
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
   
   if (mUIControlTarget != nullptr && mTarget == nullptr)
      TheSynth->AddExtraPoller(this);
   else
      TheSynth->RemoveExtraPoller(this);
}

void IModulator::Poll()
{
   if (mUIControlTarget != nullptr && mTarget == nullptr)
      mUIControlTarget->SetFromMidiCC(Value());
}

void IModulator::InitializeRange()
{
   if (mTarget != nullptr)
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

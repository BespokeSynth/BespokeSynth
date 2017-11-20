/*
  ==============================================================================

    ModulatorAdd.cpp
    Created: 19 Nov 2017 2:04:24pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorAdd.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorAdd::ModulatorAdd()
: mValue1(0)
, mValue2(0)
, mValue2Range(1)
, mMin(0)
, mMax(1)
, mTargetCable(nullptr)
, mTarget(nullptr)
, mValue1Slider(nullptr)
, mValue2Slider(nullptr)
, mValue2RangeSlider(nullptr)
{
}

void ModulatorAdd::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mValue1Slider = new FloatSlider(this, "value 1", 3, 2, 100, 15, &mValue1, 0, 1);
   mValue2Slider = new FloatSlider(this, "value 2", mValue1Slider, kAnchor_Below, 100, 15, &mValue2, -1, 1);
   mValue2RangeSlider = new FloatSlider(this, "range 2", mValue2Slider, kAnchor_Below, 100, 15, &mValue2Range, 0, 1);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorAdd::~ModulatorAdd()
{
}

void ModulatorAdd::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValue1Slider->Draw();
   mValue2Slider->Draw();
   mValue2RangeSlider->Draw();
}

void ModulatorAdd::PostRepatch(PatchCableSource* cableSource)
{
   if (mTarget != nullptr)
      mTarget->SetModulator(nullptr);
   
   if (mTargetCable->GetPatchCables().empty() == false)
   {
      mTarget = dynamic_cast<FloatSlider*>(mTargetCable->GetPatchCables()[0]->GetTarget());
      mTarget->SetModulator(this);
      mMin = mTarget->GetMin();
      mMax = mTarget->GetMax();
      mValue1 = mTarget->GetValue();
      mValue2 = 0;
      mValue1Slider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mValue1Slider->SetMode(mTarget->GetMode());
      mValue2RangeSlider->SetExtents(0, mTarget->GetMax() - mTarget->GetMin());
   }
   else
   {
      mTarget = nullptr;
   }
}

float ModulatorAdd::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mValue1 + mValue2 * mValue2Range, MIN(mMin, mMax), MAX(mMin,mMax));
}

void ModulatorAdd::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorAdd::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

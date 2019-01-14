/*
  ==============================================================================

    ModulatorAddCentered.cpp
    Created: 22 Nov 2017 9:50:17am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorAddCentered.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorAddCentered::ModulatorAddCentered()
: mValue1(0)
, mValue2(0)
, mValue2Range(1)
, mValue1Slider(nullptr)
, mValue2Slider(nullptr)
, mValue2RangeSlider(nullptr)
{
}

void ModulatorAddCentered::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mValue1Slider = new FloatSlider(this, "value 1", 3, 2, 100, 15, &mValue1, 0, 1);
   mValue2Slider = new FloatSlider(this, "value 2", mValue1Slider, kAnchor_Below, 100, 15, &mValue2, -1, 1);
   mValue2RangeSlider = new FloatSlider(this, "range 2", mValue2Slider, kAnchor_Below, 100, 15, &mValue2Range, 0, 1);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorAddCentered::~ModulatorAddCentered()
{
}

void ModulatorAddCentered::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValue1Slider->Draw();
   mValue2Slider->Draw();
   mValue2RangeSlider->Draw();
}

void ModulatorAddCentered::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   
   if (mTarget)
   {
      mValue1 = mTarget->GetValue();
      mValue2 = 0;
      mValue1Slider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mValue1Slider->SetMode(mTarget->GetMode());
      mValue2RangeSlider->SetExtents(0, mTarget->GetMax() - mTarget->GetMin());
   }
}

float ModulatorAddCentered::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mValue1 + mValue2 * mValue2Range, mTarget->GetMin(), mTarget->GetMax());
}

void ModulatorAddCentered::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModulatorAddCentered::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorAddCentered::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

/*
  ==============================================================================

    ModulatorSubtract.cpp
    Created: 9 Dec 2019 10:11:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorSubtract.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ModulatorSubtract::ModulatorSubtract()
: mValue1(0)
, mValue2(0)
, mValue1Slider(nullptr)
, mValue2Slider(nullptr)
{
}

void ModulatorSubtract::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mValue1Slider = new FloatSlider(this, "value 1", 3, 2, 100, 15, &mValue1, 0, 1);
   mValue2Slider = new FloatSlider(this, "value 2", mValue1Slider, kAnchor_Below, 100, 15, &mValue2, 0, 1);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorSubtract::~ModulatorSubtract()
{
}

void ModulatorSubtract::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValue1Slider->Draw();
   mValue2Slider->Draw();
}

void ModulatorSubtract::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   
   if (mTarget)
   {
      //mValue1 = mTarget->GetValue();
      //mValue2 = 0;
      mValue1Slider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mValue1Slider->SetMode(mTarget->GetMode());
   }
}

float ModulatorSubtract::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mValue1 - mValue2, mTarget->GetMin(), mTarget->GetMax());
}

void ModulatorSubtract::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModulatorSubtract::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorSubtract::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

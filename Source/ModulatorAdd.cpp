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
, mValue1Slider(nullptr)
, mValue2Slider(nullptr)
{
}

void ModulatorAdd::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mValue1Slider = new FloatSlider(this, "value 1", 3, 2, 100, 15, &mValue1, 0, 1);
   mValue2Slider = new FloatSlider(this, "value 2", mValue1Slider, kAnchor_Below, 100, 15, &mValue2, 0, 1);
   
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
}

void ModulatorAdd::PostRepatch(PatchCableSource* cableSource)
{
   OnModulatorRepatch();
   
   if (mTarget)
   {
      mValue1 = mTarget->GetValue();
      mValue2 = 0;
      mValue1Slider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mValue1Slider->SetMode(mTarget->GetMode());
   }
}

float ModulatorAdd::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mValue1 + mValue2, mTarget->GetMin(), mTarget->GetMax());
}

void ModulatorAdd::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
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

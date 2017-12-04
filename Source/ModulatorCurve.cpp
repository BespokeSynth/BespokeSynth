/*
  ==============================================================================

    ModulatorCurve.cpp
    Created: 29 Nov 2017 8:56:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorCurve.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "MathUtils.h"

ModulatorCurve::ModulatorCurve()
: mInput(0)
, mCurve(0)
, mInputSlider(nullptr)
, mCurveSlider(nullptr)
{
}

void ModulatorCurve::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mInputSlider = new FloatSlider(this, "input", 3, 2, 100, 15, &mInput, 0, 1);
   mCurveSlider = new FloatSlider(this, "curve", mInputSlider, kAnchor_Below, 100, 15, &mCurve, -1, 1);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorCurve::~ModulatorCurve()
{
}

void ModulatorCurve::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mInputSlider->Draw();
   mCurveSlider->Draw();
}

void ModulatorCurve::PostRepatch(PatchCableSource* cableSource)
{
   OnModulatorRepatch();
   
   if (mTarget)
   {
      mInput = mTarget->GetValue();
      mCurve = 0;
   }
}

float ModulatorCurve::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofMap(MathUtils::Curve(mInput, mCurve), 0, 1, mMin, mMax, K(clamp));
}

void ModulatorCurve::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModulatorCurve::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorCurve::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

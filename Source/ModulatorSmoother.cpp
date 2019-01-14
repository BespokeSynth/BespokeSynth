/*
  ==============================================================================

    ModulatorSmoother.cpp
    Created: 29 Nov 2017 9:35:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorSmoother.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "MathUtils.h"

ModulatorSmoother::ModulatorSmoother()
: mInput(0)
, mSmooth(.1f)
, mInputSlider(nullptr)
, mSmoothSlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

void ModulatorSmoother::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mInputSlider = new FloatSlider(this, "input", 3, 2, 100, 15, &mInput, 0, 1);
   mSmoothSlider = new FloatSlider(this, "smooth", mInputSlider, kAnchor_Below, 100, 15, &mSmooth, 0, 1);
   mSmoothSlider->SetMode(FloatSlider::kSquare);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorSmoother::~ModulatorSmoother()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorSmoother::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mInputSlider->Draw();
   mSmoothSlider->Draw();
}

void ModulatorSmoother::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   
   if (mTarget)
   {
      mInput = mTarget->GetValue();
      mInputSlider->SetExtents(mTarget->GetMin(), mTarget->GetMax());
      mInputSlider->SetMode(mTarget->GetMode());
   }
}

void ModulatorSmoother::OnTransportAdvanced(float amount)
{
   mRamp.Start(mInput, (amount * TheTransport->MsPerBar() * (mSmooth*300)));
}

float ModulatorSmoother::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mRamp.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax());
}

void ModulatorSmoother::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModulatorSmoother::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorSmoother::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

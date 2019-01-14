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

namespace
{
   const int kAdsrTime = 10000;
}

ModulatorCurve::ModulatorCurve()
: mInput(0)
, mEnvelopeControl(ofVec2f(3,19), ofVec2f(100,100))
, mInputSlider(nullptr)
{
   mEnvelopeControl.SetADSR(&mAdsr);
   mEnvelopeControl.SetViewLength(kAdsrTime);
   mEnvelopeControl.SetFixedLengthMode(true);
   mAdsr.GetFreeReleaseLevel() = true;
   mAdsr.SetNumStages(2);
   mAdsr.GetHasSustainStage() = false;
   mAdsr.GetStageData(0).target = 0;
   mAdsr.GetStageData(0).time = 0.01f;
   mAdsr.GetStageData(1).target = 1;
   mAdsr.GetStageData(1).time = kAdsrTime-.02f;
}

void ModulatorCurve::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mInputSlider = new FloatSlider(this, "input", 3, 2, 100, 15, &mInput, 0, 1);
   
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
   mEnvelopeControl.Draw();
}

void ModulatorCurve::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   
   if (mTarget)
      mInput = mTarget->GetValue();
}

float ModulatorCurve::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   mAdsr.Clear();
   mAdsr.Start(0,1);
   mAdsr.Stop(kAdsrTime);
   float val = ofClamp(mAdsr.Value(mInput * kAdsrTime), 0, 1);
   if (val != val)
      val = 0;
   return ofLerp(mTarget->GetModulatorMin(), mTarget->GetModulatorMax(), val);
}

void ModulatorCurve::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   mEnvelopeControl.OnClicked(x,y,right);
}

void ModulatorCurve::MouseReleased()
{
   IDrawableModule::MouseReleased();
   
   mEnvelopeControl.MouseReleased();
}

bool ModulatorCurve::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   
   mEnvelopeControl.MouseMoved(x, y);
   
   return false;
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

namespace
{
   const int kSaveStateRev = 1;
}

void ModulatorCurve::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mAdsr.SaveState(out);
}

void ModulatorCurve::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   mAdsr.LoadState(in);
}

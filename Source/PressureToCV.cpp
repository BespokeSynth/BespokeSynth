/*
  ==============================================================================

    PressureToCV.cpp
    Created: 28 Nov 2017 9:44:32pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PressureToCV.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PressureToCV::PressureToCV()
: mPressure(nullptr)
{
}

PressureToCV::~PressureToCV()
{
}

void PressureToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
}

void PressureToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void PressureToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PressureToCV::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mPressure = modulation.pressure;
   }
}

float PressureToCV::Value(int samplesIn)
{
   float pressure = mPressure ? mPressure->GetValue(samplesIn) : 0;
   return ofMap(pressure,0,1,GetMin(),GetMax(),K(clamped));
}

void PressureToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void PressureToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PressureToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

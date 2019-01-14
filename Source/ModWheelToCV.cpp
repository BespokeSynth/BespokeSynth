/*
  ==============================================================================

    ModWheelToCV.cpp
    Created: 28 Nov 2017 10:44:26pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModWheelToCV.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

ModWheelToCV::ModWheelToCV()
: mModWheel(nullptr)
{
}

ModWheelToCV::~ModWheelToCV()
{
}

void ModWheelToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
}

void ModWheelToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void ModWheelToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void ModWheelToCV::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mModWheel = modulation.modWheel;
   }
}

float ModWheelToCV::Value(int samplesIn)
{
   float modWheel = mModWheel ? mModWheel->GetValue(samplesIn) : 0;
   return ofMap(modWheel,0,1,GetMin(),GetMax(),K(clamped));
}

void ModWheelToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModWheelToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModWheelToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

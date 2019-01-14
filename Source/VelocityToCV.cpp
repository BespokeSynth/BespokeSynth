/*
  ==============================================================================

    VelocityToCV.cpp
    Created: 28 Nov 2017 9:44:00pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "VelocityToCV.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

VelocityToCV::VelocityToCV()
: mVelocity(0)
{
}

VelocityToCV::~VelocityToCV()
{
}

void VelocityToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
}

void VelocityToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void VelocityToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void VelocityToCV::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mVelocity = velocity;
   }
}

float VelocityToCV::Value(int samplesIn)
{
   return ofMap(mVelocity,0,127,GetMin(),GetMax(),K(clamped));
}

void VelocityToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void VelocityToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void VelocityToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

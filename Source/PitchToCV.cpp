/*
  ==============================================================================

    PitchToCV.cpp
    Created: 28 Nov 2017 9:44:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PitchToCV.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PitchToCV::PitchToCV()
: mPitch(0)
, mPitchBend(nullptr)
{
}

PitchToCV::~PitchToCV()
{
}

void PitchToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
}

void PitchToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void PitchToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PitchToCV::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = modulation.pitchBend;
   }
}

float PitchToCV::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return ofMap(mPitch+bend,0,127,GetMin(),GetMax(),K(clamped));
}

void PitchToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void PitchToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchToCV::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

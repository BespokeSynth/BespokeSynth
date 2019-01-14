/*
 ==============================================================================
 
 PitchToSpeed.cpp
 Created: 28 Nov 2017 9:44:15pm
 Author:  Ryan Challinor
 
 ==============================================================================
 */

#include "PitchToSpeed.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PitchToSpeed::PitchToSpeed()
: mPitch(0)
, mPitchBend(nullptr)
, mReferenceFreqSlider(nullptr)
, mReferenceFreq(440)
{
}

PitchToSpeed::~PitchToSpeed()
{
}

void PitchToSpeed::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mReferenceFreqSlider = new FloatSlider(this, "ref freq", 3, 2, 100, 15, &mReferenceFreq, 10, 1000);
}

void PitchToSpeed::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mReferenceFreqSlider->Draw();
}

void PitchToSpeed::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PitchToSpeed::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = modulation.pitchBend;
   }
}

float PitchToSpeed::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return TheScale->PitchToFreq(mPitch+bend) / mReferenceFreq;
}

void PitchToSpeed::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void PitchToSpeed::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchToSpeed::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}


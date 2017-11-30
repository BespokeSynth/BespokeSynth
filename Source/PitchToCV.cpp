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
   
   mMinSlider = new FloatSlider(this, "min", 3, 2, 100, 15, &mMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mMax, 0, 1);
}

void PitchToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void PitchToCV::PostRepatch(PatchCableSource* cableSource)
{
   OnModulatorRepatch();
}

void PitchToCV::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = pitchBend;
   }
}

float PitchToCV::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return ofMap(mPitch+bend,0,127,mMin,mMax,K(clamped));
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
   mTarget = dynamic_cast<FloatSlider*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
   mTargetCable->SetTarget(mTarget);
}

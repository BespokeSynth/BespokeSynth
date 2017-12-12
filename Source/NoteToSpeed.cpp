/*
  ==============================================================================

    NoteToSpeed.cpp
    Created: 3 Dec 2017 11:14:11pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteToSpeed.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToSpeed::NoteToSpeed()
: mPitch(0)
, mPitchBend(nullptr)
, mBasePitch(48)
{
}

NoteToSpeed::~NoteToSpeed()
{
}

void NoteToSpeed::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mBasePitchEntry = new TextEntry(this,"base pitch",5,3,3,&mBasePitch,0,127);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

void NoteToSpeed::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mBasePitchEntry->Draw();
}

void NoteToSpeed::PostRepatch(PatchCableSource* cableSource)
{
   OnModulatorRepatch();
}

void NoteToSpeed::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = pitchBend;
   }
}

float NoteToSpeed::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return TheScale->PitchToFreq(mPitch+bend) / TheScale->PitchToFreq(mBasePitch);
}

void NoteToSpeed::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void NoteToSpeed::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToSpeed::SetUpFromSaveData()
{
   mTargetCable->SetTarget(dynamic_cast<FloatSlider*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target"))));
}

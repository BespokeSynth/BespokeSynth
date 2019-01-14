//
//  NoteToFreq.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/17/15.
//
//

#include "NoteToFreq.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToFreq::NoteToFreq()
: mPitch(0)
, mPitchBend(nullptr)
{
}

NoteToFreq::~NoteToFreq()
{
}

void NoteToFreq::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

void NoteToFreq::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToFreq::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void NoteToFreq::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = modulation.pitchBend;
   }
}

float NoteToFreq::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return TheScale->PitchToFreq(mPitch+bend);
}

void NoteToFreq::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void NoteToFreq::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToFreq::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

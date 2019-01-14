//
//  NoteToMs.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/17/15.
//
//

#include "NoteToMs.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToMs::NoteToMs()
: mPitch(0)
, mPitchBend(nullptr)
{
}

NoteToMs::~NoteToMs()
{
}

void NoteToMs::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

void NoteToMs::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToMs::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void NoteToMs::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      mPitch = pitch;
      mPitchBend = modulation.pitchBend;
   }
}

float NoteToMs::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return 1000/TheScale->PitchToFreq(mPitch+bend);
}

void NoteToMs::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void NoteToMs::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToMs::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

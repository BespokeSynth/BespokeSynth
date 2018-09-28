/*
  ==============================================================================

    NoteToPulse.cpp
    Created: 20 Sep 2018 9:43:01pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteToPulse.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToPulse::NoteToPulse()
{
}

NoteToPulse::~NoteToPulse()
{
}

void NoteToPulse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_Pulse);
   AddPatchCableSource(mTargetCable);
}

void NoteToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToPulse::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
      DispatchPulse(mTargetCable->GetPulseReceivers(), (time - gTime) / gSampleRateMs, 0);
}

void NoteToPulse::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   /*string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;*/
}

void NoteToPulse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   //mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToPulse::SetUpFromSaveData()
{
   //mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

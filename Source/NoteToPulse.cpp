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
}

void NoteToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToPulse::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
      DispatchPulse(GetPatchCableSource(), time, velocity/127.0f, 0);
}

void NoteToPulse::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void NoteToPulse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteToPulse::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

//
//  LaunchpadNoteDisplayer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#include "LaunchpadNoteDisplayer.h"
#include "OpenFrameworksPort.h"
#include "LaunchpadKeyboard.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"

LaunchpadNoteDisplayer::LaunchpadNoteDisplayer()
: mLaunchpad(nullptr)
{
}

void LaunchpadNoteDisplayer::DrawModule()
{
   
}

void LaunchpadNoteDisplayer::DrawModuleUnclipped()
{
   DrawConnection(mLaunchpad);
}

void LaunchpadNoteDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (mLaunchpad)
      mLaunchpad->DisplayNote(pitch, velocity);
}

void LaunchpadNoteDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("gridkeyboard", moduleInfo, "", FillDropdown<LaunchpadKeyboard*>);
   
   SetUpFromSaveData();
}

void LaunchpadNoteDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mLaunchpad = dynamic_cast<LaunchpadKeyboard*>(TheSynth->FindModule(mModuleSaveData.GetString("gridkeyboard"), false));
   if (mLaunchpad)
      mLaunchpad->SetDisplayer(this);
}



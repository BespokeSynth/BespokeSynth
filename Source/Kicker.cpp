//
//  Kicker.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "Kicker.h"
#include "OpenFrameworksPort.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"

Kicker::Kicker()
{
   SetEnabled(false);
}

void Kicker::DrawModule()
{

   DrawConnection(mDrumPlayer);
   if (Minimized() || IsVisible() == false)
      return;
   
}

void Kicker::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void Kicker::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);

   if (mEnabled && mDrumPlayer)
   {
      mDrumPlayer->PlayNote(time, 3, velocity);
   }
}

void Kicker::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("enabled", moduleInfo);
   mModuleSaveData.LoadString("drumplayer", moduleInfo,"",FillDropdown<DrumPlayer*>);

   SetUpFromSaveData();
}

void Kicker::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetEnabled(mModuleSaveData.GetBool("enabled"));
   SetDrumPlayer(dynamic_cast<DrumPlayer*>(TheSynth->FindModule(mModuleSaveData.GetString("drumplayer"),false)));
}



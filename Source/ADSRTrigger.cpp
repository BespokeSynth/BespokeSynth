//
//  ADSRTrigger.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/14.
//
//

#include "ADSRTrigger.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "ADSR.h"
#include "FloatSliderLFOControl.h"

ADSRTrigger::ADSRTrigger()
: mADSRIndex(0)
{
}

void ADSRTrigger::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
}

void ADSRTrigger::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   
   if (mNoteOutput.GetHeldNotes().empty())
   {
      FloatSliderLFOControl::GetADSRControl(mADSRIndex)->Stop(time);
   }
   else if (velocity > 0)
   {
      FloatSliderLFOControl::GetADSRControl(mADSRIndex)->Start(time,1);
   }
}

void ADSRTrigger::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("global_adsr_index", moduleInfo, 0, 0, NUM_GLOBAL_ADSRS-1);
   
   SetUpFromSaveData();
}

void ADSRTrigger::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mADSRIndex = mModuleSaveData.GetInt("global_adsr_index");
}

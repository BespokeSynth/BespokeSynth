//
//  ModulationVisualizer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#include "ModulationVisualizer.h"
#include "SynthGlobals.h"
#include "ModulationChain.h"
#include "PolyphonyMgr.h"

ModulationVisualizer::ModulationVisualizer()
{
   mVoices.resize(kNumVoices);
}

void ModulationVisualizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   int y=15;
   DrawTextNormal("global:"+mGlobalModulation.GetInfoString(), 3, y);
   y += 15;
   
   for (int i=0; i<kNumVoices; ++i)
   {
      if (mVoices[i].mActive)
      {
         DrawTextNormal(ofToString(i)+":"+mVoices[i].GetInfoString(), 3, y);
         y += 15;
      }
   }
}

void ModulationVisualizer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (voiceIdx == -1)
   {
      mGlobalModulation.mActive = velocity > 0;
      mGlobalModulation.mModulators = modulation;
   }
   else
   {
      mVoices[voiceIdx].mActive = velocity > 0;
      mVoices[voiceIdx].mModulators = modulation;
   }
}

void ModulationVisualizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulationVisualizer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

string ModulationVisualizer::VizVoice::GetInfoString()
{
   string info;
   if (mModulators.pitchBend)
      info += "bend:"+ofToString(mModulators.pitchBend->GetValue(0),2)+"  ";
   if (mModulators.modWheel)
      info += "mod:"+ofToString(mModulators.modWheel->GetValue(0),2)+"  ";
   if (mModulators.pressure)
      info += "pressure:"+ofToString(mModulators.pressure->GetValue(0),2)+"  ";
   return info;
}

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
   DrawText("global:"+mGlobalModulation.GetInfoString(), 3, y);
   y += 15;
   
   for (int i=0; i<kNumVoices; ++i)
   {
      if (mVoices[i].mActive)
      {
         DrawText(ofToString(i)+":"+mVoices[i].GetInfoString(), 3, y);
         y += 15;
      }
   }
}

void ModulationVisualizer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   
   if (voiceIdx == -1)
   {
      mGlobalModulation.mActive = velocity > 0;
      mGlobalModulation.mPitchBend = pitchBend;
      mGlobalModulation.mModWheel = modWheel;
      mGlobalModulation.mPressure = pressure;
   }
   else
   {
      mVoices[voiceIdx].mActive = velocity > 0;
      mVoices[voiceIdx].mPitchBend = pitchBend;
      mVoices[voiceIdx].mModWheel = modWheel;
      mVoices[voiceIdx].mPressure = pressure;
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
   if (mPitchBend)
      info += "bend:"+ofToString(mPitchBend->GetValue(0),2)+"  ";
   if (mModWheel)
      info += "mod:"+ofToString(mModWheel->GetValue(0),2)+"  ";
   if (mPressure)
      info += "pressure:"+ofToString(mPressure->GetValue(0),2)+"  ";
   return info;
}

//
//  PreviousNote.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "PreviousNote.h"
#include "SynthGlobals.h"

PreviousNote::PreviousNote()
: mNote(-1)
, mVelocity(0)
{
}

void PreviousNote::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PreviousNote::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      return;
   }
   
   if (velocity > 0)
   {
      if (mNote != -1)
      {
         PlayNoteOutput(time, mNote, mVelocity, voiceIdx, pitchBend, modWheel, pressure);
      }
      
      mNote = pitch;
      mVelocity = velocity;
   }
   else
   {
      mNoteOutput.Flush();
   }
}

void PreviousNote::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PreviousNote::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

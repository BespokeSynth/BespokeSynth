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
: mPitch(-1)
, mVelocity(0)
{
}

void PreviousNote::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PreviousNote::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      if (mPitch != -1)
      {
         PlayNoteOutput(time, mPitch, mVelocity, voiceIdx, modulation);
      }
      
      mPitch = pitch;
      mVelocity = velocity;
   }
   else
   {
      mNoteOutput.Flush(time);
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

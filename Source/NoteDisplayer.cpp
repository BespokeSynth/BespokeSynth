//
//  NoteDisplayer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#include "NoteDisplayer.h"
#include "SynthGlobals.h"

NoteDisplayer::NoteDisplayer()
: mNote(0)
, mVelocity(0)
{
}

void NoteDisplayer::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   DrawText(NoteName(mNote) + ofToString(mNote/12 - 2) + " (" + ofToString(mNote) + ") " + ofToString(mVelocity), 4, 14);
}

void NoteDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   
   if (velocity > 0)
   {
      mNote = pitch;
      mVelocity = velocity;
   }
}

void NoteDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

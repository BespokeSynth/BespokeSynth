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
{
}

void NoteDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   list<int> notes = mNoteOutput.GetHeldNotes();
   float y = 14;
   for (var note : notes)
   {
      DrawNoteName(note, y);
      y += 13;
   }
}

void NoteDisplayer::DrawNoteName(int pitch, float y) const
{
   DrawText(NoteName(pitch) + ofToString(pitch/12 - 2) + " (" + ofToString(pitch) + ")", 4, y);
}

void NoteDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
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

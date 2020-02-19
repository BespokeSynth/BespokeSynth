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
   bzero(mVelocities, 127 * sizeof(int));
}

void NoteDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   bool* notes = mNoteOutput.GetNotes();
   float y = 14;
   for (int i=0; i<128; ++i)
   {
      if (notes[i])
      {
         DrawNoteName(i, y);
         y += 13;
      }
   }
}

void NoteDisplayer::DrawNoteName(int pitch, float y) const
{
   DrawTextNormal(NoteName(pitch) + ofToString(pitch/12 - 2) + " (" + ofToString(pitch) + ")" + " vel:"+ofToString(mVelocities[pitch]), 4, y);
}

void NoteDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   mVelocities[pitch] = velocity;
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

//
//  NoteOctaver.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/27/13.
//
//

#include "NoteOctaver.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NoteOctaver::NoteOctaver()
: mOctave(0)
, mOctaveSlider(nullptr)
{
}

void NoteOctaver::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mOctaveSlider = new IntSlider(this,"octave",4,2,100,15,&mOctave,-2,2);
}

void NoteOctaver::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mOctaveSlider->Draw();
}

void NoteOctaver::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void NoteOctaver::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      return;
   }

   if (velocity > 0)
   {
      bool found = false;
      for (list<NoteInfo>::iterator iter = mInputNotes.begin(); iter != mInputNotes.end(); ++iter)
      {
         if ((*iter).mPitch == pitch)
         {
            (*iter).mVelocity = velocity;
            (*iter).mVoiceIdx = voiceIdx;
            found = true;
            break;
         }
      }
      if (!found)
      {
         mNotesMutex.lock();
         NoteInfo note;
         note.mPitch = pitch;
         note.mVelocity = velocity;
         note.mVoiceIdx = voiceIdx;
         mInputNotes.push_back(note);
         mNotesMutex.unlock();
      }
   }
   else
   {
      for (list<NoteInfo>::iterator iter = mInputNotes.begin(); iter != mInputNotes.end(); ++iter)
      {
         if ((*iter).mPitch == pitch)
         {
            mNotesMutex.lock();
            mInputNotes.erase(iter);
            mNotesMutex.unlock();
            break;
         }
      }
   }

   pitch += mOctave * 12;
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
}

void NoteOctaver::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mOctaveSlider && mEnabled)
   {
      list<int> heldNotes = mNoteOutput.GetHeldNotes();

      mNotesMutex.lock();
      for (list<NoteInfo>::iterator iter = mInputNotes.begin(); iter != mInputNotes.end(); ++iter)
      {
         const NoteInfo& note = *iter;
         int pitch = note.mPitch + mOctave*12;
         //PlayNoteOutput fix
         PlayNoteOutput(gTime, pitch, note.mVelocity, note.mVoiceIdx);
         heldNotes.remove(pitch);
      }
      mNotesMutex.unlock();

      for (list<int>::iterator iter = heldNotes.begin(); iter != heldNotes.end(); ++iter)
         PlayNoteOutput(gTime,*iter,0, -1);
   }
}

void NoteOctaver::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteOctaver::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}



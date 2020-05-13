//
//  Capo.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/5/14.
//
//

#include "Capo.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

Capo::Capo()
: mCapo(0)
, mCapoSlider(nullptr)
{
}

void Capo::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mCapoSlider = new IntSlider(this,"capo",5,2,100,15,&mCapo,-12,12);
}

void Capo::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mCapoSlider->Draw();
}

void Capo::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void Capo::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
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
   
   pitch += mCapo;
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void Capo::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCapoSlider && mEnabled)
   {
      list<int> heldNotes = mNoteOutput.GetHeldNotesList();
      
      mNotesMutex.lock();
      for (list<NoteInfo>::iterator iter = mInputNotes.begin(); iter != mInputNotes.end(); ++iter)
      {
         const NoteInfo& note = *iter;
         int pitch = note.mPitch + mCapo;
         //PlayNoteOutput fix
         PlayNoteOutput(gTime, pitch, note.mVelocity, note.mVoiceIdx, ModulationParameters());
         heldNotes.remove(pitch);
      }
      mNotesMutex.unlock();
      
      for (list<int>::iterator iter = heldNotes.begin(); iter != heldNotes.end(); ++iter)
         PlayNoteOutput(gTime,*iter,0,-1,ModulationParameters());
   }
}

void Capo::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("capo", moduleInfo, 0, mCapoSlider);

   SetUpFromSaveData();
}

void Capo::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mCapo = mModuleSaveData.GetInt("capo");
}


//
//  ScaleDegree.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/5/16.
//
//

#include "ScaleDegree.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

ScaleDegree::ScaleDegree()
: mScaleDegree(0)
, mScaleDegreeSelector(nullptr)
{
}

void ScaleDegree::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mScaleDegreeSelector = new DropdownList(this,"degree",5,2,&mScaleDegree);
   
   mScaleDegreeSelector->AddLabel("-I", -7);
   mScaleDegreeSelector->AddLabel("-II", -6);
   mScaleDegreeSelector->AddLabel("-III", -5);
   mScaleDegreeSelector->AddLabel("-IV", -4);
   mScaleDegreeSelector->AddLabel("-V", -3);
   mScaleDegreeSelector->AddLabel("-VI", -2);
   mScaleDegreeSelector->AddLabel("-VII", -1);
   mScaleDegreeSelector->AddLabel("I", 0);
   mScaleDegreeSelector->AddLabel("II", 1);
   mScaleDegreeSelector->AddLabel("III", 2);
   mScaleDegreeSelector->AddLabel("IV", 3);
   mScaleDegreeSelector->AddLabel("V", 4);
   mScaleDegreeSelector->AddLabel("VI", 5);
   mScaleDegreeSelector->AddLabel("VII", 6);
   mScaleDegreeSelector->AddLabel("I+", 7);
}

void ScaleDegree::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mScaleDegreeSelector->Draw();
}

void ScaleDegree::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void ScaleDegree::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
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
   
   pitch = TransformPitch(pitch);
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

int ScaleDegree::TransformPitch(int pitch)
{
   int tone = TheScale->GetToneFromPitch(pitch);
   tone += mScaleDegree;
   return TheScale->GetPitchFromTone(tone);
}

void ScaleDegree::DropdownUpdated(DropdownList* slider, int oldVal)
{
   if (slider == mScaleDegreeSelector && mEnabled)
   {
      /*list<int> heldNotes = mNoteOutput.GetHeldNotesList();
      
      mNotesMutex.lock();
      for (list<NoteInfo>::iterator iter = mInputNotes.begin(); iter != mInputNotes.end(); ++iter)
      {
         const NoteInfo& note = *iter;
         int pitch = TransformPitch(note.mPitch);
         //PlayNoteOutput fix
         PlayNoteOutput(gTime, pitch, note.mVelocity, note.mVoiceIdx);
         heldNotes.remove(pitch);
      }
      mNotesMutex.unlock();
      
      for (list<int>::iterator iter = heldNotes.begin(); iter != heldNotes.end(); ++iter)
         PlayNoteOutput(gTime,*iter,0,-1);*/
      mNoteOutput.Flush(gTime);
   }
}

void ScaleDegree::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("degree", moduleInfo, 0, -7, 7);
   
   SetUpFromSaveData();
}

void ScaleDegree::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mScaleDegree = mModuleSaveData.GetInt("degree");
}

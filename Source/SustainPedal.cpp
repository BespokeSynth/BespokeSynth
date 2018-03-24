//
//  SustainPedal.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/7/14.
//
//

#include "SustainPedal.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

SustainPedal::SustainPedal()
{
   SetEnabled(false);
}

void SustainPedal::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
}

void SustainPedal::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mMutex.lock();
      if (!mEnabled)
      {
         for (auto i = mSustainedNotes.begin(); i != mSustainedNotes.end(); ++i)
            PlayNoteOutput(gTime, *i, 0, -1);
         mSustainedNotes.clear();
      }
      mMutex.unlock();
   }
}

void SustainPedal::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      mMutex.lock();
      if (velocity > 0)
      {
         if (!ListContains(pitch, mSustainedNotes)) //don't replay already-sustained notes
            PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
         mSustainedNotes.remove(pitch);   //not sustaining it if it's held down
      }
      else
      {
         if (!ListContains(pitch, mSustainedNotes))
            mSustainedNotes.push_back(pitch);
      }
      mMutex.unlock();
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void SustainPedal::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SustainPedal::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

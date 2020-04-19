/*
  ==============================================================================

    NoteLatch.cpp
    Created: 11 Apr 2020 3:28:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteLatch.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteLatch::NoteLatch()
{
   for (int i=0; i<128; ++i)
      mNoteState[i] = false;
}

void NoteLatch::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteLatch::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i=0; i<128; ++i)
      {
         if (mNoteState[i])
            PlayNoteOutput(gTime, i, 0);
      }
   }
}

void NoteLatch::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      if (velocity > 0)
      {
         if (!mNoteState[pitch])
            PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
         else
            PlayNoteOutput(time, pitch, 0, voiceIdx, modulation);
         mNoteState[pitch] = !mNoteState[pitch];
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void NoteLatch::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteLatch::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

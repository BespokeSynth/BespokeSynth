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
: mSustain(false)
{
}

void SustainPedal::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSustainCheckbox = new Checkbox(this, "sustain", 3, 3, &mSustain);
}

void SustainPedal::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mSustainCheckbox->Draw();
}

void SustainPedal::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mSustainCheckbox)
   {
      if (!mSustain)
      {
         for (int i = 0; i < 128; ++i)
         {
            if (mIsNoteBeingSustained[i])
            {
               PlayNoteOutput(gBufferSize*gInvSampleRateMs, i, 0, -1);
               mIsNoteBeingSustained[i] = false;
            }
         }
      }
   }
}

void SustainPedal::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mSustain)
   {
      if (velocity > 0)
      {
         if (!mIsNoteBeingSustained[pitch]) //don't replay already-sustained notes
            PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
         mIsNoteBeingSustained[pitch] = false;   //not being sustained by this module it if it's held down
      }
      else
      {
         mIsNoteBeingSustained[pitch] = true;
      }
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

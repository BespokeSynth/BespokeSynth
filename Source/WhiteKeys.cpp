//
//  WhiteKeys.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "WhiteKeys.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

WhiteKeys::WhiteKeys()
{
}

void WhiteKeys::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
}

void WhiteKeys::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void WhiteKeys::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   int octave = pitch / TheScale->GetTet();
   int degree = -1;
   switch (pitch % TheScale->GetTet())
   {
      case 0: degree = 0; break;
      case 2: degree = 1; break;
      case 4: degree = 2; break;
      case 5: degree = 3; break;
      case 7: degree = 4; break;
      case 9: degree = 5; break;
      case 11: degree = 6; break;
   }

   if (degree != -1)
   {
      pitch = TheScale->GetPitchFromTone(degree);
      pitch += octave * TheScale->GetTet();
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void WhiteKeys::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void WhiteKeys::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}



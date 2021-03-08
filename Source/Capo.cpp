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
   
   if (pitch >= 0 && pitch < 128)
   {
      if (velocity > 0)
      {
         mInputNotes[pitch].mOn = true;
         mInputNotes[pitch].mVelocity = velocity;
         mInputNotes[pitch].mVoiceIdx = voiceIdx;
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }
   }
   
   PlayNoteOutput(time, pitch + mCapo, velocity, voiceIdx, modulation);
}

void Capo::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCapoSlider && mEnabled)
   {
      double time = gTime + gBufferSizeMs;
      for (int pitch=0; pitch<128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(time+.01, pitch + oldVal, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
            PlayNoteOutput(time, pitch + mCapo, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         }
      }
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


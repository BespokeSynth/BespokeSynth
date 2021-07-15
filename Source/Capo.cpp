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
#include "UIControlMacros.h"

Capo::Capo()
: mCapo(0)
, mRetrigger(false)
{
}

void Capo::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   INTSLIDER(mCapoSlider,"capo",&mCapo,-12,12);
   CHECKBOX(mRetriggerCheckbox,"retrigger",&mRetrigger);
   ENDUIBLOCK(mWidth, mHeight);
}

void Capo::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mCapoSlider->Draw();
   mRetriggerCheckbox->Draw();
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
         mInputNotes[pitch].mOutputPitch = pitch + mCapo;
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }
      
      PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, velocity, mInputNotes[pitch].mVoiceIdx, modulation);
   }
}

void Capo::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCapoSlider && mEnabled && mRetrigger)
   {
      double time = gTime + gBufferSizeMs;
      for (int pitch=0; pitch<128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(time+.01, mInputNotes[pitch].mOutputPitch, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
            mInputNotes[pitch].mOutputPitch = pitch + mCapo;
            PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         }
      }
   }
}

void Capo::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Capo::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


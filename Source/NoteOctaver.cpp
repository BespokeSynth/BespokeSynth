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
   mOctaveSlider = new IntSlider(this,"octave",4,2,100,15,&mOctave,-4,4);
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
      mNoteOutput.Flush(gTime);
}

void NoteOctaver::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
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

   PlayNoteOutput(time, pitch + mOctave * 12, velocity, voiceIdx, modulation);
}

void NoteOctaver::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mOctaveSlider && mEnabled)
   {
      double time = gTime + gBufferSizeMs;
      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(time + .01, pitch + oldVal, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
            PlayNoteOutput(time, pitch + mOctave*12, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         }
      }
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



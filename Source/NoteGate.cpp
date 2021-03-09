//
//  NoteGate.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#include "NoteGate.h"
#include "SynthGlobals.h"

NoteGate::NoteGate()
: mGate(true)
{
}

NoteGate::~NoteGate()
{
}

void NoteGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGateCheckbox = new Checkbox(this, "open", 3, 4, &mGate);
}

void NoteGate::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGateCheckbox->Draw();
}

void NoteGate::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mGate || velocity == 0)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

      mActiveNotes[pitch].velocity = velocity;
      mActiveNotes[pitch].voiceIdx = voiceIdx;
      mActiveNotes[pitch].modulation = modulation;
   }

   if (!mGate)
   {
      mPendingNotes[pitch].velocity = velocity;
      mPendingNotes[pitch].voiceIdx = voiceIdx;
      mPendingNotes[pitch].modulation = modulation;
   }
}

void NoteGate::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mGateCheckbox)
   {
      double time = gTime + gBufferSizeMs;

      if (mGate)
      {
         for (int pitch = 0; pitch < 128; ++pitch)
         {
            if (mPendingNotes[pitch].velocity > 0)
            {
               PlayNoteOutput(time, pitch, mPendingNotes[pitch].velocity, mPendingNotes[pitch].voiceIdx, mPendingNotes[pitch].modulation);
               mActiveNotes[pitch].velocity = mPendingNotes[pitch].velocity;
               mActiveNotes[pitch].voiceIdx = mPendingNotes[pitch].voiceIdx;
               mActiveNotes[pitch].modulation = mPendingNotes[pitch].modulation;
               mPendingNotes[pitch].velocity = 0;
            }
         }
      }
      else
      {
         for (int pitch = 0; pitch < 128; ++pitch)
         {
            if (mActiveNotes[pitch].velocity > 0)
            {
               PlayNoteOutput(time, pitch, 0, mActiveNotes[pitch].voiceIdx, mActiveNotes[pitch].modulation);
               mPendingNotes[pitch].velocity = mActiveNotes[pitch].velocity;
               mPendingNotes[pitch].voiceIdx = mActiveNotes[pitch].voiceIdx;
               mPendingNotes[pitch].modulation = mActiveNotes[pitch].modulation;
               mActiveNotes[pitch].velocity = 0;
            }
         }
      }
   }
}

void NoteGate::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 20;
}

void NoteGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteGate::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


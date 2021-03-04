/*
  ==============================================================================

    ChordHold.cpp
    Created: 3 Mar 2021 9:56:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChordHolder.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ChordHolder::ChordHolder()
{
}

void ChordHolder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mStopButton = new ClickButton(this, "stop", 3, 3);
}

void ChordHolder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mStopButton->Draw();
}

void ChordHolder::ButtonClicked(ClickButton* button)
{
   if (button == mStopButton)
   {
      for (int i = 0; i < 128; ++i)
      {
         if (mNotePlaying[i] && !mNoteInputHeld[i])
         {
            PlayNoteOutput(gTime+gBufferSize*gInvSampleRateMs, i, 0, -1);
            mNotePlaying[i] = false;
         }
      }
   }
}

void ChordHolder::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   bool anyInputNotesHeld = false;
   for (int i = 0; i < 128; ++i)
   {
      if (mNoteInputHeld[i])
         anyInputNotesHeld = true;
   }

   if (!anyInputNotesHeld) //new input, clear any existing output
   {
      for (int i = 0; i < 128; ++i)
      {
         if (mNotePlaying[i])
         {
            PlayNoteOutput(time, i, 0, -1);
            mNotePlaying[i] = false;
         }
      }
   }

   if (velocity > 0)
   {
      if (!mNotePlaying[pitch]) //don't replay already-sustained notes
         PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      mNotePlaying[pitch] = true;

      //stop playing any voices in the chord that aren't being held anymore
      for (int i = 0; i < 128; ++i)
      {
         if (i != pitch && mNotePlaying[i] && !mNoteInputHeld[i])
         {
            PlayNoteOutput(time, i, 0, -1);
            mNotePlaying[i] = false;
         }
      }
   }

   mNoteInputHeld[pitch] = velocity > 0;
}

void ChordHolder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ChordHolder::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

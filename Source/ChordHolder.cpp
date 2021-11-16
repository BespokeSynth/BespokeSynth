/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
: mOnlyPlayWhenPulsed(false)
{
}

void ChordHolder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mStopButton = new ClickButton(this, "stop", 3, 3);
   mOnlyPlayWhenPulsedCheckbox = new Checkbox(this, "pulse to play", 40, 3, &mOnlyPlayWhenPulsed);
}

void ChordHolder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mStopButton->Draw();
   mOnlyPlayWhenPulsedCheckbox->Draw();
}

void ChordHolder::Stop()
{
   for (int i = 0; i < 128; ++i)
   {
      if (mNotePlaying[i] && !mNoteInputHeld[i])
      {
         PlayNoteOutput(gTime + gBufferSizeMs, i, 0, -1);
         mNotePlaying[i] = false;
      }
   }
}

void ChordHolder::ButtonClicked(ClickButton* button)
{
   if (button == mStopButton)
      Stop();
}

void ChordHolder::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      if (mEnabled)
      {
         for (int i = 0; i < 128; ++i)
            mNotePlaying[i] = mNoteInputHeld[i];
      }
      else
      {
         Stop();
      }
   }
}

void ChordHolder::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      if (velocity > 0)
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

         if (!mOnlyPlayWhenPulsed)
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
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }

   mNoteInputHeld[pitch] = velocity > 0;
}

void ChordHolder::OnPulse(double time, float velocity, int flags)
{
   for (int i = 0; i < 128; ++i)
   {
      if (mNotePlaying[i])
      {
         PlayNoteOutput(time, i, 0, -1);
         mNotePlaying[i] = false;
      }

      if (mNoteInputHeld[i])
      {
         PlayNoteOutput(time, i, velocity * 127, -1);
         mNotePlaying[i] = true;
      }
   }
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

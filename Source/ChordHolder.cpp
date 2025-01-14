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

void ChordHolder::Stop(double time)
{
   for (int i = 0; i < 128; ++i)
   {
      if (mNotePlaying[i] && !mNoteInputHeld[i])
      {
         PlayNoteOutput(NoteMessage(time, i, 0));
         mNotePlaying[i] = false;
      }
   }
}

void ChordHolder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mStopButton)
      Stop(time);
}

void ChordHolder::CheckboxUpdated(Checkbox* checkbox, double time)
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
         Stop(time);
      }
   }
}

void ChordHolder::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      if (note.velocity > 0)
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
                  PlayNoteOutput(NoteMessage(note.time, i, 0));
                  mNotePlaying[i] = false;
               }
            }
         }

         if (!mOnlyPlayWhenPulsed)
         {
            if (!mNotePlaying[note.pitch]) //don't replay already-sustained notes
               PlayNoteOutput(note);
            mNotePlaying[note.pitch] = true;

            //stop playing any voices in the chord that aren't being held anymore
            for (int i = 0; i < 128; ++i)
            {
               if (i != note.pitch && mNotePlaying[i] && !mNoteInputHeld[i])
               {
                  PlayNoteOutput(NoteMessage(note.time, i, 0));
                  mNotePlaying[i] = false;
               }
            }
         }
      }
   }
   else
   {
      PlayNoteOutput(note);
   }

   mNoteInputHeld[note.pitch] = note.velocity > 0;
}

void ChordHolder::OnPulse(double time, float velocity, int flags)
{
   for (int i = 0; i < 128; ++i)
   {
      if (mNotePlaying[i])
      {
         PlayNoteOutput(NoteMessage(time, i, 0));
         mNotePlaying[i] = false;
      }

      if (mNoteInputHeld[i])
      {
         PlayNoteOutput(NoteMessage(time, i, velocity * 127));
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

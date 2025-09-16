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
//
//  Monophonify.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#include "Monophonify.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

Monophonify::Monophonify()
{
   for (int i = 0; i < 128; ++i)
      mHeldNotes[i] = -1;
}

void Monophonify::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mPortamentoModeSelector, "mode", ((int*)&mPortamentoMode), 90);
   FLOATSLIDER(mGlideSlider, "glide", &mGlideTime, 0, 1000);
   ENDUIBLOCK(mWidth, mHeight);

   mPortamentoModeSelector->AddLabel("always", (int)PortamentoMode::kAlways);
   mPortamentoModeSelector->AddLabel("retrigger held", (int)PortamentoMode::kRetriggerHeld);
   mPortamentoModeSelector->AddLabel("bend held", (int)PortamentoMode::kBendHeld);
   mGlideSlider->SetMode(FloatSlider::kSquare);
}

void Monophonify::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPortamentoModeSelector->Draw();
   mGlideSlider->Draw();
}

void Monophonify::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch < 0 || note.pitch >= 128)
      return;

   mPitchBend.AppendTo(note.modulation.pitchBend);
   note.modulation.pitchBend = &mPitchBend;

   note.voiceIdx = mVoiceIdx;

   if (note.velocity > 0)
   {
      mLastVelocity = note.velocity;

      int currentlyHeldPitch = GetMostRecentCurrentlyHeldPitch();

      if (currentlyHeldPitch != -1)
      {
         if (mPortamentoMode == PortamentoMode::kBendHeld)
         {
            //just bend to the new note
            mPitchBend.RampValue(note.time, mPitchBend.GetIndividualValue(0), note.pitch - mInitialPitch, mGlideTime);
         }
         else
         {
            //trigger new note and bend
            mPitchBend.RampValue(note.time, currentlyHeldPitch - note.pitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
            PlayNoteOutput(NoteMessage(note.time, currentlyHeldPitch, 0, -1, note.modulation));
            PlayNoteOutput(note);
         }
      }
      else
      {
         if (mPortamentoMode == PortamentoMode::kAlways && mLastPlayedPitch != -1)
         {
            //bend to new note
            mPitchBend.RampValue(note.time, mLastPlayedPitch - note.pitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
         }
         else
         {
            //reset pitch bend
            mPitchBend.RampValue(note.time, 0, 0, 0);
         }

         mInitialPitch = note.pitch;
         PlayNoteOutput(note);
      }
      mLastPlayedPitch = note.pitch;
      mHeldNotes[note.pitch] = note.time;
   }
   else
   {
      bool wasCurrNote = (note.pitch == GetMostRecentCurrentlyHeldPitch());

      mHeldNotes[note.pitch] = -1;

      if (wasCurrNote)
      {
         int returnToPitch = GetMostRecentCurrentlyHeldPitch();

         if (returnToPitch != -1)
         {
            if (mPortamentoMode == PortamentoMode::kBendHeld)
            {
               //bend back to old note
               mPitchBend.RampValue(note.time, mPitchBend.GetIndividualValue(0), returnToPitch - mInitialPitch, mGlideTime);
            }
            else
            {
               //bend back to old note and retrigger
               mPitchBend.RampValue(note.time, note.pitch - returnToPitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
               PlayNoteOutput(NoteMessage(note.time, returnToPitch, mLastVelocity, note.voiceIdx, note.modulation));
            }
         }
         else
         {
            if (mPortamentoMode == PortamentoMode::kBendHeld)
            {
               //stop pitch that we started with
               PlayNoteOutput(NoteMessage(note.time, mInitialPitch, 0, note.voiceIdx, note.modulation));
            }
            else
            {
               //stop released pitch
               PlayNoteOutput(note.MakeNoteOff());
            }
         }
      }
   }
}

int Monophonify::GetMostRecentCurrentlyHeldPitch() const
{
   int mostRecentPitch = -1;
   double mostRecentTime = 0;
   for (int i = 0; i < 128; ++i)
   {
      if (mHeldNotes[i] > mostRecentTime)
      {
         mostRecentPitch = i;
         mostRecentTime = mHeldNotes[i];
      }
   }

   return mostRecentPitch;
}

void Monophonify::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
      for (int i = 0; i < 128; ++i)
         mHeldNotes[i] = -1;
   }
}

void Monophonify::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Monophonify::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voice_idx", moduleInfo, 0, 0, kNumVoices - 1);

   SetUpFromSaveData();
}

void Monophonify::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVoiceIdx = mModuleSaveData.GetInt("voice_idx");
}

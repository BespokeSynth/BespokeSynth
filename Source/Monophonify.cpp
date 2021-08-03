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
: mInitialPitch(-1)
, mLastPlayedPitch(-1)
, mLastVelocity(0)
, mVoiceIdx(0)
, mRequireHeldNote(false)
, mGlideTime(0)
, mGlideSlider(nullptr)
{
   for (int i=0; i<128; ++i)
      mHeldNotes[i] = -1;
}

void Monophonify::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   CHECKBOX(mRequireHeldNoteCheckbox, "require held", &mRequireHeldNote);
   FLOATSLIDER(mGlideSlider, "glide", &mGlideTime, 0, 1000);
   ENDUIBLOCK(mWidth, mHeight);
   
   mGlideSlider->SetMode(FloatSlider::kSquare);
}

void Monophonify::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mRequireHeldNoteCheckbox->Draw();
   mGlideSlider->Draw();
}

void Monophonify::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (pitch < 0 || pitch >= 128)
      return;
   
   mPitchBend.AppendTo(modulation.pitchBend);
   modulation.pitchBend = &mPitchBend;

   voiceIdx = mVoiceIdx;
   
   if (velocity > 0)
   {
      mLastVelocity = velocity;
      
      int bendFromPitch = GetMostRecentPitch();
      
      if (bendFromPitch != -1)
      {
         if (mRequireHeldNote)
         {
            //just bend to the new note
            mPitchBend.RampValue(time, mPitchBend.GetIndividualValue(0), pitch - mInitialPitch, mGlideTime);
         }
         else
         {
            mPitchBend.RampValue(time, bendFromPitch - pitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
            PlayNoteOutput(time, bendFromPitch, 0, -1, modulation);
            PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
         }
      }
      else
      {
         if (!mRequireHeldNote && mLastPlayedPitch != -1)
            mPitchBend.RampValue(time, mLastPlayedPitch - pitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
         else
            mPitchBend.RampValue(time, 0, 0, 0);

         mInitialPitch = pitch;
         PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      }
      mLastPlayedPitch = pitch;
      mHeldNotes[pitch] = time;
   }
   else
   {
      bool wasCurrNote = pitch == GetMostRecentPitch();
      
      mHeldNotes[pitch] = -1;
      
      int returnToPitch = GetMostRecentPitch();
      
      if (returnToPitch != -1)
      {
         if (wasCurrNote)
         {
            if (mRequireHeldNote)
            {
               //bend back to old note
               mPitchBend.RampValue(time, mPitchBend.GetIndividualValue(0), returnToPitch - mInitialPitch, mGlideTime);
            }
            else
            {
               mPitchBend.RampValue(time, pitch - returnToPitch + mPitchBend.GetIndividualValue(0), 0, mGlideTime);
               PlayNoteOutput(time, returnToPitch, mLastVelocity, voiceIdx, modulation);
            }
         }
      }
      else
      {
         PlayNoteOutput(time, mRequireHeldNote ? mInitialPitch : pitch, 0, voiceIdx, modulation);
      }
   }
}

int Monophonify::GetMostRecentPitch() const
{
   int mostRecentPitch = -1;
   double mostRecentTime = 0;
   for (int i=0; i<128; ++i)
   {
      if (mHeldNotes[i] > mostRecentTime)
      {
         mostRecentPitch = i;
         mostRecentTime = mHeldNotes[i];
      }
   }

   return mostRecentPitch;
}

void Monophonify::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(gTime);
      for (int i=0; i<128; ++i)
         mHeldNotes[i] = -1;
   }
}

void Monophonify::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Monophonify::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voice_idx", moduleInfo, 0, 0, kNumVoices-1);

   SetUpFromSaveData();
}

void Monophonify::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVoiceIdx = mModuleSaveData.GetInt("voice_idx");
}


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
{
}

void Capo::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   INTSLIDER(mCapoSlider, "capo", &mCapo, -12, 12);
   CHECKBOX(mRetriggerCheckbox, "retrigger", &mRetrigger);
   CHECKBOX(mDiatonicCheckbox, "diatonic", &mDiatonic);
   ENDUIBLOCK(mWidth, mHeight);
}

void Capo::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mCapoSlider->Draw();
   mRetriggerCheckbox->Draw();
   mDiatonicCheckbox->Draw();
}

void Capo::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void Capo::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch >= 0 && note.pitch < 128)
   {
      if (note.velocity > 0)
      {
         mInputNotes[note.pitch].mOn = true;
         mInputNotes[note.pitch].mVelocity = note.velocity;
         mInputNotes[note.pitch].mVoiceIdx = note.voiceIdx;
         mInputNotes[note.pitch].mOutputPitch = TransformPitch(note.pitch);
      }
      else
      {
         mInputNotes[note.pitch].mOn = false;
      }

      note.pitch = mInputNotes[note.pitch].mOutputPitch;
      note.voiceIdx = mInputNotes[note.pitch].mVoiceIdx;
      PlayNoteOutput(note);
   }
}

int Capo::TransformPitch(int pitch)
{
   if (mDiatonic)
   {
      pitch += mCapo;
      while (!TheScale->IsInScale(pitch))
      {
         ++pitch;
      }
      return pitch;
   }
   else
   {
      return pitch + mCapo;
   }
}

void Capo::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mCapoSlider && mEnabled && mRetrigger)
   {
      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(NoteMessage(time + .01, mInputNotes[pitch].mOutputPitch, 0, mInputNotes[pitch].mVoiceIdx));
            mInputNotes[pitch].mOutputPitch = TransformPitch(pitch);
            PlayNoteOutput(NoteMessage(time, mInputNotes[pitch].mOutputPitch, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx));
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

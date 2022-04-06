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
//  NoteCreator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#include "NoteCreator.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "PolyphonyMgr.h"
#include "UIControlMacros.h"

NoteCreator::NoteCreator()
: mTriggerButton(nullptr)
, mPitch(48)
, mVelocity(1)
, mDuration(100)
, mPitchEntry(nullptr)
, mVelocitySlider(nullptr)
, mNoteOn(false)
, mStartTime(0)
, mNoteOnCheckbox(nullptr)
, mVoiceIndex(-1)
{
}

void NoteCreator::Init()
{
   IDrawableModule::Init();
}

NoteCreator::~NoteCreator()
{
}

void NoteCreator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   TEXTENTRY_NUM(mPitchEntry, "pitch", 4, &mPitch, 0, 127);
   FLOATSLIDER(mVelocitySlider, "velocity", &mVelocity, 0, 1);
   FLOATSLIDER(mDurationSlider, "duration", &mDuration, 1, 1000);
   CHECKBOX(mNoteOnCheckbox, "on", &mNoteOn);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mTriggerButton, "trigger");
   ENDUIBLOCK(mWidth, mHeight);

   mPitchEntry->DrawLabel(true);
}

void NoteCreator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchEntry->Draw();
   mNoteOnCheckbox->Draw();
   mTriggerButton->Draw();
   mVelocitySlider->Draw();
   mDurationSlider->Draw();
}

void NoteCreator::OnPulse(double time, float velocity, int flags)
{
   TriggerNote(time, velocity * mVelocity);
}

void NoteCreator::TriggerNote(double time, float velocity)
{
   mStartTime = time;
   PlayNoteOutput(mStartTime, mPitch, velocity * 127, mVoiceIndex);
   PlayNoteOutput(mStartTime + mDuration, mPitch, 0, mVoiceIndex);
}

void NoteCreator::CheckboxUpdated(Checkbox* checkbox)
{
   double time = gTime + gBufferSizeMs;

   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
   if (checkbox == mNoteOnCheckbox)
   {
      if (mNoteOn)
      {
         PlayNoteOutput(time, mPitch, mVelocity * 127, mVoiceIndex);
      }
      else
      {
         PlayNoteOutput(time, mPitch, 0, mVoiceIndex);
         mNoteOutput.Flush(time);
      }
   }
}

void NoteCreator::ButtonClicked(ClickButton* button)
{
   if (button == mTriggerButton)
   {
      double time = gTime + gBufferSizeMs;
      TriggerNote(time, mVelocity);
   }
}

void NoteCreator::TextEntryComplete(TextEntry* entry)
{
   if (entry == mPitchEntry)
   {
      if (mNoteOn)
      {
         double time = gTime + gBufferSizeMs;
         mNoteOutput.Flush(time);
         PlayNoteOutput(time + .1f, mPitch, mVelocity * 127, mVoiceIndex);
      }
   }
}

void NoteCreator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voice index", moduleInfo, -1, -1, kNumVoices);

   SetUpFromSaveData();
}

void NoteCreator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVoiceIndex = mModuleSaveData.GetInt("voice index");
}

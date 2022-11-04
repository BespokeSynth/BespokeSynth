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
//  SmashTunes.cpp
//  Bespoke
//
//  Created by Grace Lovelace on 9/9/2022.
//
//

#include "SmashTunes.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "PolyphonyMgr.h"
#include "UIControlMacros.h"
#include <algorithm>

SmashTunes::SmashTunes()
{
}

void SmashTunes::Init()
{
   IDrawableModule::Init();
}

SmashTunes::~SmashTunes()
{
}

void SmashTunes::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 3);
   TEXTENTRY(mTextToSmashEntry, "text", 31, mTextToSmash);
   ENDUIBLOCK(mWidth, mHeight);
   UIBLOCK(3, 30, 140);
   FLOATSLIDER(mVelocitySlider, "velocity", &mVelocity, 0, 1);
   FLOATSLIDER(mDurationSlider, "duration", &mDuration, 1, 1000);
   UIBLOCK_NEWCOLUMN();
   INTSLIDER(mMinNoteSlider, "min note", &mMinNote, 0, 127);
   INTSLIDER(mMaxNoteSlider, "max note", &mMaxNote, 1, 128);
   ENDUIBLOCK(mWidth, mHeight);
}

void SmashTunes::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTextToSmashEntry->Draw();
   mMinNoteSlider->Draw();
   mMaxNoteSlider->Draw();
   mVelocitySlider->Draw();
   mDurationSlider->Draw();
}

void SmashTunes::OnPulse(double time, float velocity, int flags)
{
   TriggerNote(time, velocity * mVelocity);
}

int SmashTunes::NextNote()
{
   char currentChar = mTextToSmash[mCurrentStringIndex];
   if (currentChar == '\0')
   {
      mCurrentStringIndex = 0;
      currentChar = mTextToSmash[0];
   }

   mCurrentStringIndex++;

   return std::max(mMinNote, ((int)currentChar % mMaxNote));
}

void SmashTunes::TriggerNote(double time, float velocity)
{
   mStartTime = time;
   int nextPitch = NextNote();
   PlayNoteOutput(mStartTime, nextPitch, velocity * 127, mVoiceIndex);
   PlayNoteOutput(mStartTime + mDuration, nextPitch, 0, mVoiceIndex);
}

void SmashTunes::TextEntryComplete(TextEntry* entry)
{
   if (entry == mTextToSmashEntry)
   {
      mCurrentStringIndex = 0;
      double time = gTime + gBufferSizeMs;
      mNoteOutput.Flush(time);
      PlayNoteOutput(time + .1f, NextNote(), mVelocity * 127, mVoiceIndex);
   }
}

void SmashTunes::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voice index", moduleInfo, -1, -1, kNumVoices);

   SetUpFromSaveData();
}

void SmashTunes::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mVoiceIndex = mModuleSaveData.GetInt("voice index");
}

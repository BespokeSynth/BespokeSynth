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

    VelocityToChance.cpp
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "VelocityToChance.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

VelocityToChance::VelocityToChance()
{
   Reseed();
}

VelocityToChance::~VelocityToChance()
{
}

void VelocityToChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mFullVelocityCheckbox, "full velocity", &mFullVelocity);
   UIBLOCK_SHIFTY(5);
   INTSLIDER(mLengthSlider, "beat length", &mLength, 1, 16);
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mPrevSeedButton, "<");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mReseedButton, "*");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mNextSeedButton, ">");
   ENDUIBLOCK0();

   mSeedEntry->DrawLabel(true);
   mPrevSeedButton->PositionTo(mSeedEntry, kAnchor_Right);
   mReseedButton->PositionTo(mPrevSeedButton, kAnchor_Right);
   mNextSeedButton->PositionTo(mReseedButton, kAnchor_Right);
}

void VelocityToChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mFullVelocityCheckbox->Draw();

   if (gTime - mLastAcceptTime > 0 && gTime - mLastAcceptTime < 200)
   {
      ofPushStyle();
      ofSetColor(0, 255, 0, 255 * (1 - (gTime - mLastAcceptTime) / 200));
      ofFill();
      ofRect(106, 2, 10, 7);
      ofPopStyle();
   }

   if (gTime - mLastRejectTime > 0 && gTime - mLastRejectTime < 200)
   {
      ofPushStyle();
      ofSetColor(255, 0, 0, 255 * (1 - (gTime - mLastRejectTime) / 200));
      ofFill();
      ofRect(106, 9, 10, 7);
      ofPopStyle();
   }

   mLengthSlider->SetShowing(mDeterministic);
   mLengthSlider->Draw();
   mSeedEntry->SetShowing(mDeterministic);
   mSeedEntry->Draw();
   mPrevSeedButton->SetShowing(mDeterministic);
   mPrevSeedButton->Draw();
   mReseedButton->SetShowing(mDeterministic);
   mReseedButton->Draw();
   mNextSeedButton->SetShowing(mDeterministic);
   mNextSeedButton->Draw();

   if (mDeterministic)
   {
      ofRectangle lengthRect = mLengthSlider->GetRect(true);
      ofPushStyle();
      ofSetColor(0, 255, 0);
      ofFill();
      float pos = fmod(TheTransport->GetMeasureTime(gTime) * TheTransport->GetTimeSigTop() / mLength, 1);
      const float kPipSize = 3;
      float moduleWidth, moduleHeight;
      GetModuleDimensions(moduleWidth, moduleHeight);
      ofRect(ofMap(pos, 0, 1, 0, moduleWidth - kPipSize), lengthRect.y - 5, kPipSize, kPipSize);
      ofPopStyle();
   }
}

void VelocityToChance::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   float random;
   if (mDeterministic)
   {
      const int kStepResolution = 128;
      uint64_t step = int(TheTransport->GetMeasureTime(note.time) * kStepResolution);
      int randomIndex = step % ((mLength * kStepResolution) / TheTransport->GetTimeSigTop());
      random = ((abs(DeterministicRandom(mSeed + note.pitch * 13, randomIndex)) % 10000) / 10000.0f);
   }
   else
   {
      random = ofRandom(1);
   }

   bool accept = (random <= note.velocity / 127.0f);
   if (accept)
   {
      if (mFullVelocity)
         note.velocity = 127;
      PlayNoteOutput(note);
   }

   if (note.velocity > 0)
   {
      if (accept)
         mLastAcceptTime = note.time;
      else
         mLastRejectTime = note.time;
   }
   else
   {
      PlayNoteOutput(note.MakeNoteOff());
   }
}

void VelocityToChance::Reseed()
{
   mSeed = gRandom() % 10000;
}

void VelocityToChance::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPrevSeedButton)
      mSeed = (mSeed - 1 + 10000) % 10000;
   if (button == mReseedButton)
      Reseed();
   if (button == mNextSeedButton)
      mSeed = (mSeed + 1) % 10000;
}

void VelocityToChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = mDeterministic ? 60 : 20;
}

void VelocityToChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("deterministic", moduleInfo, false);

   SetUpFromSaveData();
}

void VelocityToChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));

   mDeterministic = mModuleSaveData.GetBool("deterministic");
}

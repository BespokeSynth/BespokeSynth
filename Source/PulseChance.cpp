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

    PulseChance.cpp
    Created: 4 Feb 2020 12:17:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseChance.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Transport.h"

PulseChance::PulseChance()
{
   Reseed();
}

PulseChance::~PulseChance()
{
}

void PulseChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mChanceSlider, "chance", &mChance, 0, 1);
   CHECKBOX(mDeterministicCheckbox, "deterministic", &mDeterministic);
   UIBLOCK_SHIFTY(5);
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

void PulseChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mChanceSlider->Draw();
   mDeterministicCheckbox->Draw();

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

   mSeedEntry->SetShowing(mDeterministic);
   mSeedEntry->Draw();
   mPrevSeedButton->SetShowing(mDeterministic);
   mPrevSeedButton->Draw();
   mReseedButton->SetShowing(mDeterministic);
   mReseedButton->Draw();
   mNextSeedButton->SetShowing(mDeterministic);
   mNextSeedButton->Draw();
}

void PulseChance::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (!mEnabled)
   {
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
      return;
   }

   if (flags & kPulseFlag_Reset)
      mRandomIndex = 0;

   float random;
   if (mDeterministic)
   {
      random = ((abs(DeterministicRandom(mSeed, mRandomIndex)) % 10000) / 10000.0f);
      ++mRandomIndex;
   }
   else
   {
      random = ofRandom(1);
   }

   bool accept = random <= mChance;
   if (accept)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);

   if (accept)
      mLastAcceptTime = gTime;
   else
      mLastRejectTime = gTime;
}

void PulseChance::Reseed()
{
   mSeed = gRandom() % 10000;
}

void PulseChance::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPrevSeedButton)
      mSeed = (mSeed - 1 + 10000) % 10000;
   if (button == mReseedButton)
      Reseed();
   if (button == mNextSeedButton)
      mSeed = (mSeed + 1) % 10000;
}

void PulseChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = mDeterministic ? 61 : 38;
}

void PulseChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

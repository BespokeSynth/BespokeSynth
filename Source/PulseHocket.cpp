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

    PulseHocket.cpp
    Created: 22 Feb 2020 10:40:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseHocket.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"

PulseHocket::PulseHocket()
{
   mWeight[0] = 1;
   for (int i = 1; i < kMaxDestinations; ++i)
      mWeight[i] = 0;

   Reseed();
}

PulseHocket::~PulseHocket()
{
}

void PulseHocket::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      FLOATSLIDER(mWeightSlider[i], ("weight " + ofToString(i)).c_str(), &mWeight[i], 0, 1);
   }
   UIBLOCK_SHIFTY(5);
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mPrevSeedButton, "<");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mReseedButton, "*");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mNextSeedButton, ">");
   ENDUIBLOCK(mWidth, mHeight);
   mWidth = 121;

   GetPatchCableSource()->SetEnabled(false);
   mSeedEntry->DrawLabel(true);
   mPrevSeedButton->PositionTo(mSeedEntry, kAnchor_Right);
   mReseedButton->PositionTo(mPrevSeedButton, kAnchor_Right);
   mNextSeedButton->PositionTo(mReseedButton, kAnchor_Right);
}

void PulseHocket::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
   {
      mWeightSlider[i]->SetShowing(i < mNumDestinations);
      mWeightSlider[i]->Draw();
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

void PulseHocket::AdjustHeight()
{
   float deterministicPad = 28;

   if (!mDeterministic)
      deterministicPad = 3;

   float height = mNumDestinations * 17 + deterministicPad;
   mSeedEntry->Move(0, height - mHeight);
   mPrevSeedButton->Move(0, height - mHeight);
   mReseedButton->Move(0, height - mHeight);
   mNextSeedButton->Move(0, height - mHeight);
   mHeight = height;
}

void PulseHocket::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (flags & kPulseFlag_Reset)
      mRandomIndex = 0;

   float totalWeight = 0;
   for (int i = 0; i < mNumDestinations; ++i)
      totalWeight += mWeight[i];
   float random;
   if (mDeterministic)
   {
      random = ((abs(DeterministicRandom(mSeed, mRandomIndex)) % 10000) / 10000.0f) * totalWeight;
      ++mRandomIndex;
   }
   else
   {
      random = ofRandom(totalWeight);
   }

   int selectedDestination;
   for (selectedDestination = 0; selectedDestination < mNumDestinations; ++selectedDestination)
   {
      if (random <= mWeight[selectedDestination] || selectedDestination == mNumDestinations - 1)
         break;
      random -= mWeight[selectedDestination];
   }

   DispatchPulse(mDestinationCables[selectedDestination], time, velocity, flags);
}

void PulseHocket::Reseed()
{
   mSeed = gRandom() % 10000;
}

void PulseHocket::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPrevSeedButton)
      mSeed = (mSeed - 1 + 10000) % 10000;
   if (button == mReseedButton)
      Reseed();
   if (button == mNextSeedButton)
      mSeed = (mSeed + 1) % 10000;
}

void PulseHocket::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("num_outputs", moduleInfo, 5, 2, kMaxDestinations, K(isTextField));
   mModuleSaveData.LoadBool("deterministic", moduleInfo, false);

   SetUpFromSaveData();
}

void PulseHocket::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));

   mNumDestinations = mModuleSaveData.GetInt("num_outputs");
   int oldNumItems = (int)mDestinationCables.size();
   if (mNumDestinations > oldNumItems)
   {
      for (int i = oldNumItems; i < mNumDestinations; ++i)
      {
         mDestinationCables.push_back(new PatchCableSource(this, kConnectionType_Pulse));
         mDestinationCables[i]->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
         AddPatchCableSource(mDestinationCables[i]);
         ofRectangle rect = mWeightSlider[i]->GetRect(true);
         mDestinationCables[i]->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
      }
   }
   else if (mNumDestinations < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= mNumDestinations; --i)
      {
         RemovePatchCableSource(mDestinationCables[i]);
      }
      mDestinationCables.resize(mNumDestinations);
   }

   mDeterministic = mModuleSaveData.GetBool("deterministic");

   AdjustHeight();
}

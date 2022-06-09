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

    NoteChance.cpp
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteChance.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

NoteChance::NoteChance()
{
   Reseed();
}

NoteChance::~NoteChance()
{
}

void NoteChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mChanceSlider, "chance", &mChance, 0, 1);
   UIBLOCK_SHIFTY(5);
   TEXTENTRY_NUM(mLengthEntry, "beat length", 3, &mLength, 1, 128);
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mReseedButton, "reseed");
   ENDUIBLOCK0();

   mLengthEntry->DrawLabel(true);
   mSeedEntry->DrawLabel(true);
   mReseedButton->PositionTo(mSeedEntry, kAnchor_Right);
}

void NoteChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mChanceSlider->Draw();

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

   mLengthEntry->SetShowing(mDeterministic);
   mLengthEntry->Draw();
   mSeedEntry->SetShowing(mDeterministic);
   mSeedEntry->Draw();
   mReseedButton->SetShowing(mDeterministic);
   mReseedButton->Draw();

   if (mDeterministic)
   {
      ofRectangle lengthRect = mLengthEntry->GetRect(true);
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

void NoteChance::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (velocity > 0)
      ComputeSliders(0);

   float random;
   if (mDeterministic)
   {
      const int kStepResolution = 128;
      uint64_t step = int(TheTransport->GetMeasureTime(time) * kStepResolution);
      int randomIndex = step % ((mLength * kStepResolution) / TheTransport->GetTimeSigTop());
      random = ((abs(DeterministicRandom(mSeed, randomIndex)) % 10000) / 10000.0f);
   }
   else
   {
      random = ofRandom(1);
   }

   bool accept = random <= mChance;
   if (accept || velocity == 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (velocity > 0)
   {
      if (accept)
         mLastAcceptTime = time;
      else
         mLastRejectTime = time;
   }
}

void NoteChance::Reseed()
{
   mSeed = gRandom() % 10000;
}

void NoteChance::ButtonClicked(ClickButton* button)
{
   if (button == mReseedButton)
      Reseed();
}

void NoteChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = mDeterministic ? 60 : 20;
}

void NoteChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("deterministic", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));

   mDeterministic = mModuleSaveData.GetBool("deterministic");
}

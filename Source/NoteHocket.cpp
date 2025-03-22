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

    NoteHocket.cpp
    Created: 19 Dec 2019 10:40:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteHocket.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

NoteHocket::NoteHocket()
{
   for (int i = 0; i < 128; ++i)
      mLastNoteDestinations[i] = -1;
   for (int i = 0; i < kMaxDestinations; ++i)
      mWeight[i] = (i == 0) ? 1 : 0;

   Reseed();
}

void NoteHocket::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      FLOATSLIDER(mWeightSlider[i], ("weight " + ofToString(i)).c_str(), &mWeight[i], 0, 1);
   }
   UIBLOCK_SHIFTY(5);
   INTSLIDER(mLengthSlider, "beat length", &mLength, 1, 16);
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

void NoteHocket::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
   {
      mWeightSlider[i]->SetShowing(i < mNumDestinations);
      mWeightSlider[i]->Draw();
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

void NoteHocket::AdjustHeight()
{
   float deterministicPad = 45;

   if (!mDeterministic)
      deterministicPad = 3;

   float height = mNumDestinations * 17 + deterministicPad;
   mLengthSlider->Move(0, height - mHeight);
   mSeedEntry->Move(0, height - mHeight);
   mPrevSeedButton->Move(0, height - mHeight);
   mReseedButton->Move(0, height - mHeight);
   mNextSeedButton->Move(0, height - mHeight);
   mHeight = height;
}

void NoteHocket::PlayNote(NoteMessage note)
{
   int selectedDestination = 0;
   if (note.velocity > 0)
   {
      ComputeSliders(0);

      float totalWeight = 0;
      for (int i = 0; i < mNumDestinations; ++i)
         totalWeight += mWeight[i];
      float random;
      if (mDeterministic)
      {
         const int kStepResolution = 128;
         uint64_t step = int(TheTransport->GetMeasureTime(note.time) * kStepResolution);
         int randomIndex = step % ((mLength * kStepResolution) / TheTransport->GetTimeSigTop());
         random = ((abs(DeterministicRandom(mSeed, randomIndex)) % 10000) / 10000.0f) * totalWeight;
      }
      else
      {
         random = ofRandom(totalWeight);
      }

      for (selectedDestination = 0; selectedDestination < mNumDestinations; ++selectedDestination)
      {
         if (random <= mWeight[selectedDestination] || selectedDestination == mNumDestinations - 1)
            break;
         random -= mWeight[selectedDestination];
      }

      if (mLastNoteDestinations[note.pitch] != -1 && mLastNoteDestinations[note.pitch] != selectedDestination)
         SendNoteToIndex(mLastNoteDestinations[note.pitch], note.MakeNoteOff());
      mLastNoteDestinations[note.pitch] = selectedDestination;
   }
   else
   {
      selectedDestination = mLastNoteDestinations[note.pitch];
      if (selectedDestination == -1)
         return;
      mLastNoteDestinations[note.pitch] = -1;
   }

   SendNoteToIndex(selectedDestination, note);
}

void NoteHocket::SendNoteToIndex(int index, NoteMessage note)
{
   mDestinationCables[index]->PlayNoteOutput(note);
}

void NoteHocket::Reseed()
{
   mSeed = gRandom() % 10000;
}

void NoteHocket::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteHocket::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPrevSeedButton)
      mSeed = (mSeed - 1 + 10000) % 10000;
   if (button == mReseedButton)
      Reseed();
   if (button == mNextSeedButton)
      mSeed = (mSeed + 1) % 10000;
}

void NoteHocket::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_outputs", moduleInfo, 5, 2, kMaxDestinations, K(isTextField));
   mModuleSaveData.LoadBool("deterministic", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteHocket::SetUpFromSaveData()
{
   mNumDestinations = mModuleSaveData.GetInt("num_outputs");
   int oldNumItems = (int)mDestinationCables.size();
   if (mNumDestinations > oldNumItems)
   {
      for (int i = oldNumItems; i < mNumDestinations; ++i)
      {
         mDestinationCables.push_back(new AdditionalNoteCable());
         mDestinationCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
         mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
         AddPatchCableSource(mDestinationCables[i]->GetPatchCableSource());
         ofRectangle rect = mWeightSlider[i]->GetRect(true);
         mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
      }
   }
   else if (mNumDestinations < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= mNumDestinations; --i)
      {
         RemovePatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      }
      mDestinationCables.resize(mNumDestinations);
   }

   mDeterministic = mModuleSaveData.GetBool("deterministic");

   AdjustHeight();
}

void NoteHocket::SaveLayout(ofxJSONElement& moduleInfo)
{
}

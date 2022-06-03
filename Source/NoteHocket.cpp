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
   mWeight[0] = 1;
   for (int i = 1; i < kMaxDestinations; ++i)
      mWeight[i] = 0;
   for (int i = 0; i < 128; ++i)
      mLastNoteDestinations[i] = -1;

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
   TEXTENTRY_NUM(mLengthEntry, "beat length", 3, &mLength, 1, 128);
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mReseedButton, "reseed");
   ENDUIBLOCK(mWidth, mHeight);
   mWidth += 20;

   GetPatchCableSource()->SetEnabled(false);
   mLengthEntry->DrawLabel(true);
   mSeedEntry->DrawLabel(true);
   mReseedButton->PositionTo(mSeedEntry, kAnchor_Right);
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

   mLengthEntry->SetShowing(mDeterministic);
   mLengthEntry->Draw();
   mSeedEntry->SetShowing(mDeterministic);
   mSeedEntry->Draw();
   mReseedButton->SetShowing(mDeterministic);
   mReseedButton->Draw();
}

void NoteHocket::AdjustHeight()
{
   float deterministicPad = 45;

   if (!mDeterministic)
      deterministicPad = 3;

   float height = mNumDestinations * 17 + deterministicPad;
   mLengthEntry->Move(0, height - mHeight);
   mSeedEntry->Move(0, height - mHeight);
   mReseedButton->Move(0, height - mHeight);
   mHeight = height;
}

void NoteHocket::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int selectedDestination = 0;
   if (velocity > 0)
   {
      ComputeSliders(0);

      float totalWeight = 0;
      for (int i = 0; i < mNumDestinations; ++i)
         totalWeight += mWeight[i];
      float random;
      if (mDeterministic)
      {
         const int kStepResolution = 128;
         uint64_t step = int(TheTransport->GetMeasureTime(time) * kStepResolution);
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

      if (mLastNoteDestinations[pitch] != -1 && mLastNoteDestinations[pitch] != selectedDestination)
         SendNoteToIndex(mLastNoteDestinations[pitch], time, pitch, 0, voiceIdx, modulation);
      mLastNoteDestinations[pitch] = selectedDestination;
   }
   else
   {
      selectedDestination = mLastNoteDestinations[pitch];
      if (selectedDestination == -1)
         return;
      mLastNoteDestinations[pitch] = -1;
   }

   SendNoteToIndex(selectedDestination, time, pitch, velocity, voiceIdx, modulation);
}

void NoteHocket::SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   mDestinationCables[index]->PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteHocket::Reseed()
{
   mSeed = gRandom() % 10000;
}

void NoteHocket::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteHocket::ButtonClicked(ClickButton* button)
{
   if (button == mReseedButton)
      Reseed();
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
         mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0));
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
   IDrawableModule::SaveLayout(moduleInfo);
}

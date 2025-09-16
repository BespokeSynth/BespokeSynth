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
//  RandomNoteGenerator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#include "RandomNoteGenerator.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"

RandomNoteGenerator::RandomNoteGenerator()
{
}

void RandomNoteGenerator::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void RandomNoteGenerator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this, "interval", 5, 2, ((int*)(&mInterval)));
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mProbabilitySlider = new FloatSlider(this, "probability", 5, 20, 100, 15, &mProbability, 0, 1);
   mPitchSlider = new IntSlider(this, "pitch", 5, 38, 100, 15, &mPitch, 0, 127);
   mVelocitySlider = new FloatSlider(this, "velocity", 5, 56, 100, 15, &mVelocity, 0, 1);
   mOffsetSlider = new FloatSlider(this, "offset", 5, 74, 100, 15, &mOffset, -1, 1);
   mSkipSlider = new IntSlider(this, "skip", mIntervalSelector, kAnchor_Right, 60, 15, &mSkip, 1, 10);
}

RandomNoteGenerator::~RandomNoteGenerator()
{
   TheTransport->RemoveListener(this);
}

void RandomNoteGenerator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mIntervalSelector->Draw();
   mSkipSlider->Draw();
   mProbabilitySlider->Draw();
   mPitchSlider->Draw();
   mVelocitySlider->Draw();
   mOffsetSlider->Draw();
}

void RandomNoteGenerator::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   ++mSkipCount;

   mNoteOutput.Flush(time);
   if (mSkipCount >= mSkip)
   {
      mSkipCount = 0;
      if (mProbability >= ofRandom(1))
         PlayNoteOutput(NoteMessage(time, mPitch, mVelocity * 127));
   }
}

void RandomNoteGenerator::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void RandomNoteGenerator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mOffsetSlider)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
      {
         transportListenerInfo->mInterval = mInterval;
         transportListenerInfo->mOffsetInfo = OffsetInfo(mOffset / TheTransport->CountInStandardMeasure(mInterval), !K(offsetIsInMs));
      }
   }
}

void RandomNoteGenerator::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mPitchSlider)
      mNoteOutput.Flush(time);
}

void RandomNoteGenerator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
      {
         transportListenerInfo->mInterval = mInterval;
         transportListenerInfo->mOffsetInfo = OffsetInfo(mOffset / TheTransport->CountInStandardMeasure(mInterval), !K(offsetIsInMs));
      }
   }
}

void RandomNoteGenerator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void RandomNoteGenerator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

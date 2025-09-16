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

    NoteCounter.cpp
    Created: 24 Apr 2021 3:47:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteCounter.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteCounter::NoteCounter()
{
}

void NoteCounter::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);

   Reseed();
}

void NoteCounter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float desiredWidth = 116;

   UIBLOCK(3, 3, desiredWidth - 6);
   DROPDOWN(mIntervalSelector, "interval", ((int*)(&mInterval)), 50);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mSyncCheckbox, "sync", &mSync);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_PUSHSLIDERWIDTH(75);
   INTSLIDER(mCustomDivisorSlider, "div", &mCustomDivisor, 1, 32);
   UIBLOCK_POPSLIDERWIDTH();
   UIBLOCK_NEWLINE();
   INTSLIDER(mStartSlider, "start", &mStart, 0, 32);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, 32);
   CHECKBOX(mRandomCheckbox, "random", &mRandom);
   ENDUIBLOCK(mWidth, mHeight);

   UIBLOCK(3, mHeight + 5, desiredWidth - 6);
   INTSLIDER(mDeterministicLengthSlider, "beat length", &mDeterministicLength, 1, 16);
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mPrevSeedButton, "<");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mReseedButton, "*");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mNextSeedButton, ">");
   ENDUIBLOCK0();

   mWidth = desiredWidth;

   mSeedEntry->DrawLabel(true);
   mPrevSeedButton->PositionTo(mSeedEntry, kAnchor_Right);
   mReseedButton->PositionTo(mPrevSeedButton, kAnchor_Right);
   mNextSeedButton->PositionTo(mReseedButton, kAnchor_Right);

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
   mIntervalSelector->AddLabel("div", kInterval_CustomDivisor);
}

NoteCounter::~NoteCounter()
{
   TheTransport->RemoveListener(this);
}

void NoteCounter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mIntervalSelector->Draw();
   mSyncCheckbox->Draw();
   mStartSlider->Draw();
   mLengthSlider->Draw();
   mRandomCheckbox->Draw();
   mCustomDivisorSlider->SetShowing(mInterval == kInterval_CustomDivisor);
   mCustomDivisorSlider->Draw();

   mDeterministicLengthSlider->SetShowing(mDeterministic && mRandom);
   mDeterministicLengthSlider->Draw();
   mSeedEntry->SetShowing(mDeterministic && mRandom);
   mSeedEntry->Draw();
   mPrevSeedButton->SetShowing(mDeterministic && mRandom);
   mPrevSeedButton->Draw();
   mReseedButton->SetShowing(mDeterministic && mRandom);
   mReseedButton->Draw();
   mNextSeedButton->SetShowing(mDeterministic && mRandom);
   mNextSeedButton->Draw();

   if (mDeterministic && mRandom)
   {
      ofRectangle lengthRect = mDeterministicLengthSlider->GetRect(true);
      ofPushStyle();
      ofSetColor(0, 255, 0);
      ofFill();
      float pos = fmod(TheTransport->GetMeasureTime(gTime) * TheTransport->GetTimeSigTop() / mDeterministicLength, 1);
      const float kPipSize = 3;
      float moduleWidth, moduleHeight;
      GetModuleDimensions(moduleWidth, moduleHeight);
      ofRect(ofMap(pos, 0, 1, 0, moduleWidth - kPipSize), lengthRect.y - 5, kPipSize, kPipSize);
      ofPopStyle();
   }
}

void NoteCounter::Step(double time, float velocity, int pulseFlags)
{
   if (!mEnabled)
      return;

   bool sync = mSync || pulseFlags & kPulseFlag_SyncToTransport;

   if (sync)
   {
      mStep = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mLength);
   }
   else
   {
      if (pulseFlags & kPulseFlag_Reset)
         mStep = 0;
      if (pulseFlags & kPulseFlag_Random)
         mStep = GetRandom(time, 999) % mLength;
      if (pulseFlags & kPulseFlag_Align)
      {
         int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
         int numMeasures = ceil(float(mLength) / stepsPerMeasure);
         int measure = TheTransport->GetMeasure(time) % numMeasures;
         int step = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % mLength;
         mStep = step;
      }
   }

   mNoteOutput.Flush(time);
   if (mRandom)
      PlayNoteOutput(NoteMessage(time, GetRandom(time, 0) % mLength + mStart, 127));
   else
      PlayNoteOutput(NoteMessage(time, mStep + mStart, 127));

   if (!sync)
   {
      int direction = 1;
      if (pulseFlags & kPulseFlag_Backward)
         direction = -1;
      if (pulseFlags & kPulseFlag_Repeat)
         direction = 0;
      mStep = (mStep + mLength + direction) % mLength;
   }
}

std::uint64_t NoteCounter::GetRandom(double time, int seedOffset) const
{
   std::uint64_t random;
   if (mDeterministic && mRandom)
   {
      const int kStepResolution = 128;
      uint64_t step = int(TheTransport->GetMeasureTime(time) * kStepResolution);
      int randomIndex = step % ((mDeterministicLength * kStepResolution) / TheTransport->GetTimeSigTop());
      random = abs(DeterministicRandom(mSeed + seedOffset, randomIndex));
   }
   else
   {
      random = gRandom();
   }

   return random;
}

void NoteCounter::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void NoteCounter::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;

   Step(time, velocity, flags);
}

void NoteCounter::Reseed()
{
   mSeed = gRandom() % 10000;
}

void NoteCounter::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPrevSeedButton)
      mSeed = (mSeed - 1 + 10000) % 10000;
   if (button == mReseedButton)
      Reseed();
   if (button == mNextSeedButton)
      mSeed = (mSeed + 1) % 10000;
}

void NoteCounter::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
      mStep = 0;
   }
}

void NoteCounter::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mCustomDivisorSlider)
      mTransportListenerInfo->mCustomDivisor = mCustomDivisor;
}

void NoteCounter::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void NoteCounter::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
   if (mDeterministic && mRandom)
      height += 40;
   if (mCustomDivisorSlider->IsShowing())
      width += 60;
}

void NoteCounter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("deterministic_random", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteCounter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mDeterministic = mModuleSaveData.GetBool("deterministic_random");
}

void NoteCounter::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mHasExternalPulseSource;
}

void NoteCounter::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev == 1)
   {
      float dummy;
      in >> dummy; //width
      in >> dummy; //height
   }
   in >> mHasExternalPulseSource;
}

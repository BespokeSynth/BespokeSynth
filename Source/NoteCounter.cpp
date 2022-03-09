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
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteCounter::NoteCounter()
: mInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mStart(0)
, mLength(16)
, mStep(0)
, mCustomDivisor(8)
, mRandom(false)
{
}

void NoteCounter::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void NoteCounter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", ((int*)(&mInterval)), 50); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mSyncCheckbox, "sync", &mSync); UIBLOCK_NEWLINE();
   INTSLIDER(mStartSlider, "start", &mStart, 0, 32);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, 32);
   CHECKBOX(mRandomCheckbox, "random", &mRandom);
   INTSLIDER(mCustomDivisorSlider, "div", &mCustomDivisor, 1, 32);
   ENDUIBLOCK(mWidth, mHeight);
   
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
         mStep = gRandom() % mLength;
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
      PlayNoteOutput(time, gRandom() % mLength + mStart, 127, -1);
   else
      PlayNoteOutput(time, mStep + mStart, 127, -1);

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

void NoteCounter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(gTime);
      mStep = 0;
   }
}

void NoteCounter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCustomDivisorSlider)
      mTransportListenerInfo->mCustomDivisor = mCustomDivisor;
}

void NoteCounter::DropdownUpdated(DropdownList* list, int oldVal)
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
   if (!mCustomDivisorSlider->IsShowing())
      height -= 17;
}

void NoteCounter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteCounter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

namespace
{
   const int kSaveStateRev = 1;
}

void NoteCounter::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mWidth;
   out << mHeight;
   out << mHasExternalPulseSource;
}

void NoteCounter::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return;  //this was saved before we added versioning, bail out
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   in >> mWidth;
   in >> mHeight;
   in >> mHasExternalPulseSource;
}

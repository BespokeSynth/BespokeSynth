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
{
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
   mCustomDivisorSlider->SetShowing(mInterval == kInterval_CustomDivisor);
   mCustomDivisorSlider->Draw();
}

void NoteCounter::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;
   
   if (mSync)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int measure = TheTransport->GetMeasure(time);
      mStep = (TheTransport->GetQuantized(time, mTransportListenerInfo) + measure * stepsPerMeasure) % mLength;
   }
   else
   {
      mStep = (mStep + 1) % mLength;
   }
   
   mNoteOutput.Flush(time);
   PlayNoteOutput(time, mStep + mStart, 127, -1);
}

void NoteCounter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
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

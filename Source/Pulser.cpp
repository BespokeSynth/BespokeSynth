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

    Sequencer.cpp
    Created: 17 Oct 2018 9:38:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Pulser.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Pulser::Pulser()
: mInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mTimeMode(kTimeMode_Step)
, mTimeModeSelector(nullptr)
, mWaitingForDownbeat(false)
, mFreeTimeStep(30)
, mFreeTimeSlider(nullptr)
, mFreeTimeCounter(0)
, mOffset(0)
, mOffsetSlider(nullptr)
, mRandomStep(false)
, mRandomStepCheckbox(nullptr)
, mResetLength(8)
, mCustomDivisor(8)
{
}

void Pulser::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheTransport->AddAudioPoller(this);
}

void Pulser::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",75,2,(int*)(&mInterval));
   mTimeModeSelector = new DropdownList(this,"timemode",5,2,(int*)(&mTimeMode));
   mFreeTimeSlider = new FloatSlider(this,"t",75,2,44,15,&mFreeTimeStep,10,1000,0);
   mOffsetSlider = new FloatSlider(this,"offset",mTimeModeSelector,kAnchor_Below,113,15,&mOffset,-1,1);
   mRandomStepCheckbox = new Checkbox(this,"random",mOffsetSlider,kAnchor_Below,&mRandomStep);
   mResetLengthSlider = new IntSlider(this, "reset", mRandomStepCheckbox, kAnchor_Below, 113, 15, &mResetLength, 1, 16);
   mCustomDivisorSlider = new IntSlider(this, "div", mRandomStepCheckbox, kAnchor_Right, 52, 15, &mCustomDivisor, 1, 32);
   
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
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
   mIntervalSelector->AddLabel("none", kInterval_None);
   mIntervalSelector->AddLabel("div", kInterval_CustomDivisor);
   
   mTimeModeSelector->AddLabel("step", kTimeMode_Step);
   mTimeModeSelector->AddLabel("reset", kTimeMode_Reset);
   mTimeModeSelector->AddLabel("sync", kTimeMode_Sync);
   mTimeModeSelector->AddLabel("align", kTimeMode_Align);
   mTimeModeSelector->AddLabel("downbeat", kTimeMode_Downbeat);
   mTimeModeSelector->AddLabel("dnbeat2", kTimeMode_Downbeat2);
   mTimeModeSelector->AddLabel("dnbeat4", kTimeMode_Downbeat4);
   mTimeModeSelector->AddLabel("free", kTimeMode_Free);
   
   mFreeTimeSlider->SetMode(FloatSlider::kSquare);
   
   mFreeTimeSlider->SetShowing(mTimeMode == kTimeMode_Free);
}

Pulser::~Pulser()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void Pulser::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mIntervalSelector->Draw();
   mTimeModeSelector->Draw();
   mFreeTimeSlider->Draw();
   mOffsetSlider->Draw();
   mRandomStepCheckbox->Draw();
   mResetLengthSlider->Draw();
   mCustomDivisorSlider->SetShowing(mInterval == kInterval_CustomDivisor);
   mCustomDivisorSlider->Draw();
}

void Pulser::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      if (mEnabled && (mTimeMode == kTimeMode_Downbeat || mTimeMode == kTimeMode_Downbeat2 || mTimeMode == kTimeMode_Downbeat4))
         mWaitingForDownbeat = true;
   }
}

void Pulser::OnTransportAdvanced(float amount)
{
   PROFILER(Pulser);
   
   ComputeSliders(0);
   
   if (mTimeMode == kTimeMode_Free)
   {
      float ms = amount * TheTransport->MsPerBar();
      mFreeTimeCounter += ms;
      if (mFreeTimeCounter > mFreeTimeStep)
      {
         OnTimeEvent(gTime + (mFreeTimeCounter - mFreeTimeStep));
         mFreeTimeCounter -= mFreeTimeStep;
      }
   }
}

void Pulser::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   float offsetMs = GetOffset()*TheTransport->MsPerBar();
   
   int flags = 0;
   
   bool shouldReset = false;
   if (mTimeMode == kTimeMode_Downbeat)
      shouldReset = TheTransport->GetQuantized(time, mTransportListenerInfo) == 0;
   if (mTimeMode == kTimeMode_Downbeat2)           
      shouldReset = TheTransport->GetQuantized(time, mTransportListenerInfo) == 0 && TheTransport->GetMeasure(time+offsetMs) % 2 == 0;
   if (mTimeMode == kTimeMode_Downbeat4)           
      shouldReset = TheTransport->GetQuantized(time, mTransportListenerInfo) == 0 && TheTransport->GetMeasure(time+offsetMs) % 4 == 0;
   if (mTimeMode == kTimeMode_Reset)
   {
      int step = 0;
      if (TheTransport->GetMeasureFraction(mInterval) < 1)
      {
         step = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo);
      }
      else
      {
         int measure = TheTransport->GetMeasure(time);
         step = measure / TheTransport->GetMeasureFraction(mInterval);
      }
      if (step % mResetLength == 0)
         shouldReset = true;
   }
   
   if (shouldReset)
      flags |= kPulseFlag_Reset;
   
   if (mRandomStep)
      flags |= kPulseFlag_Random;
   
   if (mTimeMode == kTimeMode_Sync)
      flags |= kPulseFlag_SyncToTransport;
   
   if (mTimeMode == kTimeMode_Align)
      flags |= kPulseFlag_Align;
   
   if (mWaitingForDownbeat && shouldReset)
      mWaitingForDownbeat = false;
   
   if (mWaitingForDownbeat && (mTimeMode == kTimeMode_Downbeat || mTimeMode == kTimeMode_Downbeat2 || mTimeMode == kTimeMode_Downbeat4))
      return;
   
   DispatchPulse(GetPatchCableSource(), time, 1, flags);
}

void Pulser::GetModuleDimensions(float& width, float& height)
{
   width = 121;
   height = 52;
   if (mTimeMode == kTimeMode_Reset)
      height += 18;
}

void Pulser::ButtonClicked(ClickButton* button)
{
}

float Pulser::GetOffset()
{
   if (mInterval == kInterval_None)
      return 0;
   if (mInterval == kInterval_2)
      return -mOffset * 2;
   if (mInterval == kInterval_3)
      return -mOffset * 3;
   if (mInterval == kInterval_4)
      return -mOffset * 4;
   if (mInterval == kInterval_8)
      return -mOffset * 8;
   if (mInterval == kInterval_16)
      return -mOffset * 16;
   if (mInterval == kInterval_CustomDivisor)
      return -mOffset / mCustomDivisor;
   return (-mOffset/TheTransport->CountInStandardMeasure(mInterval));
}

void Pulser::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
      {
         transportListenerInfo->mInterval = mInterval;
         transportListenerInfo->mOffsetInfo = OffsetInfo(GetOffset(), false);
      }
   }
   if (list == mTimeModeSelector)
   {
      mIntervalSelector->SetShowing(mTimeMode != kTimeMode_Free);
      mFreeTimeSlider->SetShowing(mTimeMode == kTimeMode_Free);
      
      if (mTimeMode == kTimeMode_Free && mInterval < kInterval_None)
      {
         mFreeTimeStep = TheTransport->GetDuration(mInterval);
         TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
         if (transportListenerInfo != nullptr)
            transportListenerInfo->mInterval = kInterval_None;
      }
      else if (oldVal == kTimeMode_Free)
      {
         TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
         if (transportListenerInfo != nullptr)
         {
            transportListenerInfo->mInterval = mInterval;
            transportListenerInfo->mOffsetInfo = OffsetInfo(GetOffset(), false);
         }
      }
      
      mResetLengthSlider->SetShowing(mTimeMode == kTimeMode_Reset);
   }
}

void Pulser::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
   {
      if (mTimeMode != kTimeMode_Free)
      {
         TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
         if (transportListenerInfo != nullptr)
         {
            transportListenerInfo->mInterval = mInterval;
            transportListenerInfo->mOffsetInfo = OffsetInfo(GetOffset(), false);
         }
      }
   }
}

void Pulser::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mCustomDivisorSlider)
      mTransportListenerInfo->mCustomDivisor = mCustomDivisor;
}

void Pulser::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void Pulser::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void Pulser::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

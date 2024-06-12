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

    PulseDelayer.cpp
    Created: 15 Feb 2020 2:53:22pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseDelayer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Profiler.h"

PulseDelayer::PulseDelayer()
{
}

void PulseDelayer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

PulseDelayer::~PulseDelayer()
{
   TheTransport->RemoveAudioPoller(this);
}

void PulseDelayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mDelaySlider = new FloatSlider(this, "delay", 4, 4, 100, 15, &mDelay, 0, 1, 4);
}

void PulseDelayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDelaySlider->Draw();

   float t = (gTime - mLastPulseTime) / (mDelay * TheTransport->GetDuration(kInterval_1n));
   if (t > 0 && t < 1)
   {
      ofPushStyle();
      ofNoFill();
      ofCircle(54, 11, 10);
      ofFill();
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      ofCircle(54 + sin(t * TWO_PI) * 10, 11 - cos(t * TWO_PI) * 10, 2);
      ofPopStyle();
   }
}

void PulseDelayer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (!mEnabled)
      mConsumeIndex = mAppendIndex; //effectively clears the queue
}

void PulseDelayer::OnTransportAdvanced(float amount)
{
   PROFILER(PulseDelayer);

   ComputeSliders(0);

   int end = mAppendIndex;
   if (mAppendIndex < mConsumeIndex)
      end += kQueueSize;
   for (int i = mConsumeIndex; i < end; ++i)
   {
      const PulseInfo& info = mInputPulses[i % kQueueSize];
      if (gTime + TheTransport->GetEventLookaheadMs() >= info.mTriggerTime)
      {
         DispatchPulse(GetPatchCableSource(), info.mTriggerTime, info.mVelocity, info.mFlags);
         mConsumeIndex = (mConsumeIndex + 1) % kQueueSize;
      }
   }
}

void PulseDelayer::OnPulse(double time, float velocity, int flags)
{
   if (!mEnabled)
   {
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
      return;
   }

   if (velocity > 0)
      mLastPulseTime = time;

   if ((mAppendIndex + 1) % kQueueSize != mConsumeIndex)
   {
      PulseInfo info;
      info.mVelocity = velocity;
      info.mTriggerTime = time + mDelay / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      info.mFlags = flags;
      mInputPulses[mAppendIndex] = info;
      mAppendIndex = (mAppendIndex + 1) % kQueueSize;
   }
}

void PulseDelayer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void PulseDelayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PulseDelayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

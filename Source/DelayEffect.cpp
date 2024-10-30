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
//  DelayEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "DelayEffect.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "ModularSynth.h"

#include "juce_core/juce_core.h"

DelayEffect::DelayEffect()
: mDelayBuffer(DELAY_BUFFER_SIZE)
{
}

void DelayEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mDelaySlider, "delay", &mDelay, GetMinDelayMs(), 1000);
   FLOATSLIDER(mFeedbackSlider, "amount", &mFeedback, 0, 1);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 45);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mShortTimeCheckbox, "short", &mShortTime);
   UIBLOCK_NEWLINE();
   CHECKBOX(mDryCheckbox, "dry", &mDry);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mEchoCheckbox, "feedback", &mEcho);
   UIBLOCK_NEWLINE();
   CHECKBOX(mAcceptInputCheckbox, "input", &mAcceptInput);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mInvertCheckbox, "invert", &mInvert);
   ENDUIBLOCK(mWidth, mHeight);

   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8nd", kInterval_8nd);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16nd", kInterval_16nd);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

void DelayEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DelayEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();
   mDelayBuffer.SetNumChannels(buffer->NumActiveChannels());

   if (mInterval != kInterval_None)
   {
      mDelay = TheTransport->GetDuration(mInterval) + .1f; //+1 to avoid perfect sample collision
      mDelayRamp.Start(time, mDelay, time + 10);
   }

   mAmountRamp.Start(time, mFeedback, time + 3);
   for (int i = 0; i < bufferSize; ++i)
   {
      mFeedback = mAmountRamp.Value(time);

      ComputeSliders(i);

      float delay;
      if (mDelaySlider->GetModulator() != nullptr)
         delay = MAX(mDelay, GetMinDelayMs());
      else
         delay = MAX(mDelayRamp.Value(time), GetMinDelayMs());

      float delaySamps = delay / gInvSampleRateMs;
      if (mFeedbackModuleMode)
         delaySamps -= gBufferSize;
      delaySamps = ofClamp(delaySamps, 0.1f, DELAY_BUFFER_SIZE - 2);

      int sampsAgoA = int(delaySamps);
      int sampsAgoB = sampsAgoA + 1;

      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float sample = mDelayBuffer.GetSample(sampsAgoA, ch);
         float nextSample = mDelayBuffer.GetSample(sampsAgoB, ch);
         float a = delaySamps - sampsAgoA;
         float delayedSample = (1 - a) * sample + a * nextSample; //interpolate

         float in = buffer->GetChannel(ch)[i];

         if (!mEcho && mAcceptInput) //single delay, no continuous feedback so do it pre
            mDelayBuffer.Write(buffer->GetChannel(ch)[i], ch);

         float delayInput = delayedSample * mFeedback * (mInvert ? -1 : 1);
         JUCE_UNDENORMALISE(delayInput);
         if (!std::isnan(delayInput))
            buffer->GetChannel(ch)[i] += delayInput;

         if (mEcho && mAcceptInput) //continuous feedback so do it post
            mDelayBuffer.Write(buffer->GetChannel(ch)[i], ch);

         if (!mAcceptInput)
            mDelayBuffer.Write(delayInput, ch);

         if (!mDry)
            buffer->GetChannel(ch)[i] -= in;
      }

      time += gInvSampleRateMs;
   }
}

void DelayEffect::DrawModule()
{
   if (!mEnabled)
      return;

   mDelaySlider->Draw();
   mFeedbackSlider->Draw();
   mIntervalSelector->Draw();
   mShortTimeCheckbox->Draw();
   mDryCheckbox->Draw();
   mEchoCheckbox->Draw();
   mAcceptInputCheckbox->Draw();
   mInvertCheckbox->Draw();
}

float DelayEffect::GetEffectAmount()
{
   if (!mEnabled || !mAcceptInput)
      return 0;
   return mFeedback;
}

void DelayEffect::SetDelay(float delay)
{
   mDelay = delay;
   mDelayRamp.Start(gTime, mDelay, gTime + 10);
   mInterval = kInterval_None;
}

void DelayEffect::SetShortMode(bool on)
{
   mShortTime = on;
   mDelaySlider->SetExtents(GetMinDelayMs(), mShortTime ? 20 : 1000);
}

void DelayEffect::SetFeedbackModuleMode()
{
   mFeedbackModuleMode = true;

   mDry = false;
   mEcho = false;
   mInvert = true;
   SetShortMode(true);
   SetDelay(20);

   mDryCheckbox->SetShowing(false);
   mEchoCheckbox->SetShowing(false);
   mAcceptInputCheckbox->SetPosition(mAcceptInputCheckbox->GetPosition(true).x, mAcceptInputCheckbox->GetPosition(true).y - 17);
   mInvertCheckbox->SetPosition(mInvertCheckbox->GetPosition(true).x, mInvertCheckbox->GetPosition(true).y - 17);
   mHeight -= 17;
}

float DelayEffect::GetMinDelayMs() const
{
   if (mFeedbackModuleMode)
      return (gBufferSize + 1) * gInvSampleRateMs;
   return .1f;
}

void DelayEffect::SetEnabled(bool enabled)
{
   mEnabled = enabled;
   if (!enabled)
      mDelayBuffer.ClearBuffer();
}

void DelayEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mShortTimeCheckbox)
      SetShortMode(mShortTime);
   if (checkbox == mEnabledCheckbox)
   {
      if (!mEnabled)
         mDelayBuffer.ClearBuffer();
   }
}

void DelayEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mDelaySlider)
   {
      mInterval = kInterval_None;
      mDelayRamp.Start(time, mDelay, time + 30);
   }
}

void DelayEffect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void DelayEffect::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mDelayBuffer.SaveState(out);
}

void DelayEffect::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mDelayBuffer.LoadState(in);
}

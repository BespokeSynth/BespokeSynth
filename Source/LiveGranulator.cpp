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
//  LiveGranulator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 10/2/13.
//
//

#include "LiveGranulator.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "UIControlMacros.h"

LiveGranulator::LiveGranulator()
: mBufferLength(gSampleRate * 5)
, mBuffer(mBufferLength)
{
   mGranulator.SetLiveMode(true);
   mGranulator.mSpeed = 1;
   mGranulator.mGrainOverlap = 12;
   mGranulator.mGrainLengthMs = 300;
}

void LiveGranulator::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_None, OffsetInfo(0, true), false);
}

namespace
{
   const float kBufferWidth = 80;
   const float kBufferHeight = 65;
}

void LiveGranulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(80);
   FLOATSLIDER(mGranOverlap, "overlap", &mGranulator.mGrainOverlap, .5f, MAX_GRAINS);
   FLOATSLIDER(mGranSpeed, "speed", &mGranulator.mSpeed, -3, 3);
   FLOATSLIDER(mGranLengthMs, "len ms", &mGranulator.mGrainLengthMs, 1, 1000);
   FLOATSLIDER(mDrySlider, "dry", &mDry, 0, 1);
   DROPDOWN(mAutoCaptureDropdown, "autocapture", (int*)(&mAutoCaptureInterval), 45);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER(mGranPosRandomize, "pos r", &mGranulator.mPosRandomizeMs, 0, 200);
   FLOATSLIDER(mGranSpeedRandomize, "spd r", &mGranulator.mSpeedRandomize, 0, .3f);
   FLOATSLIDER(mGranSpacingRandomize, "spa r", &mGranulator.mSpacingRandomize, 0, 1);
   CHECKBOX(mFreezeCheckbox, "frz", &mFreeze);
   UIBLOCK_SHIFTX(35);
   CHECKBOX(mGranOctaveCheckbox, "g oct", &mGranulator.mOctaves);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mWidthSlider, "width", &mGranulator.mWidth, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mBufferX = mWidth + 3;
   mWidth += kBufferWidth + 3 * 2;

   UIBLOCK(mBufferX, mHeight - 17, kBufferWidth);
   FLOATSLIDER(mPosSlider, "pos", &mPos, -gSampleRate, gSampleRate);
   ENDUIBLOCK0();

   mAutoCaptureDropdown->AddLabel("none", kInterval_None);
   mAutoCaptureDropdown->AddLabel("4n", kInterval_4n);
   mAutoCaptureDropdown->AddLabel("8n", kInterval_8n);
   mAutoCaptureDropdown->AddLabel("16n", kInterval_16n);

   mGranPosRandomize->SetMode(FloatSlider::kSquare);
   mGranSpeedRandomize->SetMode(FloatSlider::kSquare);
   mGranLengthMs->SetMode(FloatSlider::kSquare);
}

LiveGranulator::~LiveGranulator()
{
   TheTransport->RemoveListener(this);
}

void LiveGranulator::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(LiveGranulator);

   float bufferSize = buffer->BufferSize();
   mBuffer.SetNumChannels(buffer->NumActiveChannels());

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);

      mGranulator.SetLiveMode(!mFreeze);
      if (!mFreeze)
      {
         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            mBuffer.Write(buffer->GetChannel(ch)[i], ch);
      }
      else if (mFreezeExtraSamples < FREEZE_EXTRA_SAMPLES_COUNT)
      {
         ++mFreezeExtraSamples;
         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            mBuffer.Write(buffer->GetChannel(ch)[i], ch);
      }

      if (mEnabled)
      {
         float sample[ChannelBuffer::kMaxNumChannels];
         Clear(sample, ChannelBuffer::kMaxNumChannels);
         mGranulator.ProcessFrame(time, mBuffer.GetRawBuffer(), mBufferLength, mBuffer.GetRawBufferOffset(0) - mFreezeExtraSamples - 1 + mPos, 1.0f, sample);
         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            buffer->GetChannel(ch)[i] = mDry * buffer->GetChannel(ch)[i] + sample[ch];
      }

      time += gInvSampleRateMs;
   }
}

void LiveGranulator::DrawModule()
{
   if (!mEnabled)
      return;

   mGranOverlap->Draw();
   mGranSpeed->Draw();
   mGranLengthMs->Draw();
   mGranPosRandomize->Draw();
   mGranSpeedRandomize->Draw();
   mFreezeCheckbox->Draw();
   mGranOctaveCheckbox->Draw();
   mPosSlider->Draw();
   mDrySlider->Draw();
   mAutoCaptureDropdown->Draw();
   mGranSpacingRandomize->Draw();
   mWidthSlider->Draw();
   if (mEnabled)
   {
      int drawLength = MIN(mBufferLength, gSampleRate * 2);
      if (mFreeze)
         drawLength = MIN(mBufferLength, drawLength + mFreezeExtraSamples);
      mBuffer.Draw(mBufferX, 3, kBufferWidth, kBufferHeight, drawLength);
      mGranulator.Draw(mBufferX, 3 + 20, kBufferWidth, kBufferHeight - 20 * 2, mBuffer.GetRawBufferOffset(0) - drawLength, drawLength, mBufferLength);
   }
}

float LiveGranulator::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp(.5f + fabsf(mGranulator.mSpeed - 1), 0, 1);
}

void LiveGranulator::Freeze()
{
   mFreeze = true;
   mFreezeExtraSamples = 0;
}

void LiveGranulator::OnTimeEvent(double time)
{
   Freeze();
}

void LiveGranulator::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mBuffer.ClearBuffer();
   if (checkbox == mFreezeCheckbox)
   {
      mFreezeExtraSamples = 0;
      if (mFreeze)
      {
         mEnabled = true;
         Freeze();
      }
   }
}

void LiveGranulator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mAutoCaptureDropdown)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mAutoCaptureInterval;
      if (mAutoCaptureInterval == kInterval_None)
      {
         mFreeze = false;
         mFreezeExtraSamples = 0;
      }
   }
}

void LiveGranulator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mPosSlider)
   {
      if (!mFreeze)
         mPos = MIN(mPos, 0);
   }
}

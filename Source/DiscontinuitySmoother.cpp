/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    DiscontinuitySmoother.cpp
    Created: 30 Sept 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "DiscontinuitySmoother.h"
#include "SynthGlobals.h"
#include "Profiler.h"

DiscontinuitySmoother::DiscontinuitySmoother()
: IAudioProcessor(gBufferSize)
, IDrawableModule(90, 90)
, mDrawOffsetBuffer(gSampleRate * 0.1f)
{
}

void DiscontinuitySmoother::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mThresholdSlider = new FloatSlider(this, "threshold", 5, 2, 110, 15, &mThreshold, 0.0f, 1.0f);
   mRampLengthSlider = new FloatSlider(this, "ramp length", 5, 20, 110, 15, &mRampLength, 0.0f, 500);

   mThresholdSlider->SetMode(FloatSlider::kSquare);
}

DiscontinuitySmoother::~DiscontinuitySmoother()
{
}

void DiscontinuitySmoother::Process(double time)
{
   PROFILER(DiscontinuitySmoother);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();
   mDrawOffsetBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());

   ChannelBuffer* out = target->GetBuffer();
   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);
      for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
      {
         float sample = GetBuffer()->GetChannel(ch)[i];
         if (mEnabled && fabsf(sample - mLastSample[ch]) > mThreshold)
         {
            mSwitchAndRamp.StartSwitch();
            mLastDiscontinuityTime = time + i * gInvSampleRateMs;
         }
         mLastSample[ch] = sample;
      }
      for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
      {
         float sample = GetBuffer()->GetChannel(ch)[i];
         float outputSample = mSwitchAndRamp.Process(ch, sample, 1.0f / mRampLength);

         mDrawOffsetBuffer.Write(mSwitchAndRamp.GetOffset(ch), ch);
         out->GetChannel(ch)[i] += outputSample;
         GetVizBuffer()->Write(outputSample, ch);
      }
   }

   GetBuffer()->Reset();
}

void DiscontinuitySmoother::DrawModule()
{
   mThresholdSlider->Draw();
   mRampLengthSlider->Draw();

   float x = 5;
   float y = 40;
   float w = GetRect().width - 15;
   float h = 50;

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {

      int samplesPerPixel = mDrawOffsetBuffer.Size() / w;
      ofPushStyle();
      ofSetLineWidth(2);
      ofBeginShape();
      for (int i = 0; i < w; ++i)
         ofVertex(x + w - i, y + h / 2 + (h / 2) * mDrawOffsetBuffer.GetSample(i * samplesPerPixel, ch));
      ofEndShape();
      ofPopStyle();
   }

   ofPushStyle();
   ofFill();
   ofSetColor(0, 255, 0, 255 * (1 - std::clamp((gTime - mLastDiscontinuityTime) / 500.0, 0.0, 1.0)));
   ofRect(x + w, y + h / 2 - 4, 8, 8);
   ofPopStyle();
}

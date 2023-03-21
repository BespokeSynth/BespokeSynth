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
//  GateEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/19/13.
//
//

#include "GateEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

GateEffect::GateEffect()
{
}

void GateEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mThresholdSlider = new FloatSlider(this, "threshold", 5, 2, 110, 15, &mThreshold, 0, 1);
   mAttackSlider = new FloatSlider(this, "attack", 5, 18, 110, 15, &mAttackTime, .1f, 500);
   mReleaseSlider = new FloatSlider(this, "release", 5, 34, 110, 15, &mReleaseTime, .1f, 500);

   mThresholdSlider->SetMode(FloatSlider::kSquare);
}

void GateEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(GateEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   for (int i = 0; i < bufferSize; ++i)
   {
      const float decayTime = .01f;
      float scalar = powf(0.5f, 1.0f / (decayTime * gSampleRate));
      float input = 0;
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         input = MAX(input, fabsf(buffer->GetChannel(ch)[i]));

      if (input >= mPeak)
      {
         /* When we hit a peak, ride the peak to the top. */
         mPeak = input;
      }
      else
      {
         /* Exponential decay of output when signal is low. */
         mPeak = mPeak * scalar;
         if (mPeak < FLT_EPSILON)
            mPeak = 0.0;
      }

      if (mPeak >= mThreshold && mEnvelope < 1)
         mEnvelope = MIN(1, mEnvelope + gInvSampleRateMs / mAttackTime);
      if (mPeak < mThreshold && mEnvelope > 0)
         mEnvelope = MAX(0, mEnvelope - gInvSampleRateMs / mReleaseTime);

      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mEnvelope;

      time += gInvSampleRateMs;
   }
}

void GateEffect::DrawModule()
{
   mThresholdSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();

   ofPushStyle();
   ofFill();
   ofSetColor(0, 255, 0, gModuleDrawAlpha * .4f);
   ofRect(5, 2, 110 * sqrtf(mPeak), 7);
   ofSetColor(255, 0, 0, gModuleDrawAlpha * .4f);
   ofRect(5, 9, 110 * mEnvelope, 7);
   ofPopStyle();
}

void GateEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void GateEffect::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void GateEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

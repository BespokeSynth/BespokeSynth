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
//  SampleVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#include "SampleVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"

SampleVoice::SampleVoice(IDrawableModule* owner)
: mOwner(owner)
{
}

SampleVoice::~SampleVoice()
{
}

bool SampleVoice::IsDone(double time)
{
   return mAdsr.IsDone(time);
}

bool SampleVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(SampleVoice);

   if (IsDone(time) ||
       mVoiceParams->mSampleData == nullptr ||
       mVoiceParams->mSampleLength == 0)
      return false;

   float volSq = mVoiceParams->mVol * mVoiceParams->mVol;

   for (int pos = 0; pos < out->BufferSize(); ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);

      if (mPos <= mVoiceParams->mSampleLength || mVoiceParams->mLoop)
      {
         float freq = TheScale->PitchToFreq(GetPitch(pos));
         float speed;
         if (mVoiceParams->mDetectedFreq != -1)
            speed = freq / mVoiceParams->mDetectedFreq;
         else
            speed = freq / TheScale->PitchToFreq(TheScale->ScaleRoot() + 48);

         float sample = GetInterpolatedSample(mPos, mVoiceParams->mSampleData, mVoiceParams->mSampleLength) * mAdsr.Value(time) * volSq;

         if (out->NumActiveChannels() == 1)
         {
            out->GetChannel(0)[pos] += sample;
         }
         else
         {
            out->GetChannel(0)[pos] += sample * GetLeftPanGain(GetPan());
            out->GetChannel(1)[pos] += sample * GetRightPanGain(GetPan());
         }

         mPos += speed;
      }

      time += gInvSampleRateMs;
   }

   return true;
}

void SampleVoice::Start(double time, float target)
{
   mPos = 0;
   mAdsr.Start(time, target, mVoiceParams->mAdsr);
}

void SampleVoice::Stop(double time)
{
   mAdsr.Stop(time);
}

void SampleVoice::ClearVoice()
{
   mAdsr.Clear();
}

void SampleVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<SampleVoiceParams*>(params);
}

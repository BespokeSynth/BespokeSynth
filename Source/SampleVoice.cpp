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
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"
#include "Sample.h"

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
       mVoiceParams->mSample == nullptr ||
       mVoiceParams->mSample->Data() == nullptr)
      return false;

   float volSq = mVoiceParams->mVol * mVoiceParams->mVol;

   for (int pos = 0; pos < out->BufferSize(); ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);

      int stopSample = mVoiceParams->mSample->LengthInSamples();
      if (mVoiceParams->mStopSample != -1)
         stopSample = mVoiceParams->mStopSample;

      int jumpFrom = -1;
      int jumpTo = -1;
      bool isSustaining = !mAdsr.IsDone(time);
      if (isSustaining && mVoiceParams->mSustainLoopStart != -1 && mVoiceParams->mSustainLoopEnd != -1)
      {
         jumpFrom = mVoiceParams->mSustainLoopEnd;
         jumpTo = mVoiceParams->mSustainLoopStart;
      }

      if (mPos <= stopSample)
      {
         float freq = TheScale->PitchToFreq(GetPitch(pos));
         float speed = freq / TheScale->PitchToFreq(mVoiceParams->mSamplePitch);

         for (int i = 0; i < 2; ++i)
         {
            int ch = MIN(i, mVoiceParams->mSample->Data()->NumActiveChannels() - 1);
            float sample = GetInterpolatedSample(mPos, mVoiceParams->mSample->Data()->GetChannel(ch), mVoiceParams->mSample->LengthInSamples()) * mAdsr.Value(time) * volSq;
            float pan = i == 0 ? GetLeftPanGain(GetPan()) : GetRightPanGain(GetPan());
            out->GetChannel(i)[pos] += sample * pan;
         }

         mPos += speed;

         if (jumpFrom != -1 && mPos >= jumpFrom)
            mPos += (jumpTo - jumpFrom);
      }

      time += gInvSampleRateMs;
   }

   return true;
}

void SampleVoice::Start(double time, float target)
{
   mPos = mVoiceParams->mStartSample;
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

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
//  FMVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#include "FMVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"
#include "PolyphonyMgr.h"

FMVoice::FMVoice(IDrawableModule* owner)
: mOwner(owner)
{
}

FMVoice::~FMVoice()
{
}

bool FMVoice::IsDone(double time)
{
   return mOsc.GetADSR()->IsDone(time);
}

bool FMVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(FMVoice);

   if (IsDone(time))
      return false;

   int bufferSize = out->BufferSize();
   int channels = out->NumActiveChannels();
   double sampleIncrementMs = gInvSampleRateMs;
   ChannelBuffer* destBuffer = out;

   if (oversampling != 1)
   {
      gMidiVoiceWorkChannelBuffer.SetNumActiveChannels(channels);
      destBuffer = &gMidiVoiceWorkChannelBuffer;
      gMidiVoiceWorkChannelBuffer.Clear();
      bufferSize *= oversampling;
      sampleIncrementMs /= oversampling;
   }

   for (int pos = 0; pos < bufferSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos / oversampling);

      float oscFreq = TheScale->PitchToFreq(GetPitch(pos / oversampling));
      float harmFreq = oscFreq * mHarm.GetADSR()->Value(time) * mVoiceParams->mHarmRatio;
      float harmFreq2 = harmFreq * mHarm2.GetADSR()->Value(time) * mVoiceParams->mHarmRatio2;

      float harmPhaseInc2 = GetPhaseInc(harmFreq2) / oversampling;

      mHarmPhase2 += harmPhaseInc2;
      while (mHarmPhase2 > FTWO_PI)
      {
         mHarmPhase2 -= FTWO_PI;
      }

      float modHarmFreq = harmFreq + mHarm2.Audio(time, mHarmPhase2 + mVoiceParams->mPhaseOffset2) * harmFreq2 * mModIdx2.Value(time) * mVoiceParams->mModIdx2;

      float harmPhaseInc = GetPhaseInc(modHarmFreq) / oversampling;

      mHarmPhase += harmPhaseInc;
      while (mHarmPhase > FTWO_PI)
      {
         mHarmPhase -= FTWO_PI;
      }

      float modOscFreq = oscFreq + mHarm.Audio(time, mHarmPhase + mVoiceParams->mPhaseOffset1) * harmFreq * mModIdx.Value(time) * mVoiceParams->mModIdx;
      float oscPhaseInc = GetPhaseInc(modOscFreq) / oversampling;

      mOscPhase += oscPhaseInc;
      while (mOscPhase > FTWO_PI)
      {
         mOscPhase -= FTWO_PI;
      }

      float sample = mOsc.Audio(time, mOscPhase + mVoiceParams->mPhaseOffset0) * mVoiceParams->mVol / 20.0f;
      if (channels == 1)
      {
         destBuffer->GetChannel(0)[pos] += sample;
      }
      else
      {
         destBuffer->GetChannel(0)[pos] += sample * GetLeftPanGain(GetPan());
         destBuffer->GetChannel(1)[pos] += sample * GetRightPanGain(GetPan());
      }

      time += sampleIncrementMs;
   }

   if (oversampling != 1)
   {
      //assume power-of-two
      while (oversampling > 1)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            for (int ch = 0; ch < channels; ++ch)
               destBuffer->GetChannel(ch)[i] = (destBuffer->GetChannel(ch)[i * 2] + destBuffer->GetChannel(ch)[i * 2 + 1]) / 2;
         }
         oversampling /= 2;
         bufferSize /= 2;
      }

      for (int ch = 0; ch < channels; ++ch)
         Add(out->GetChannel(ch), destBuffer->GetChannel(ch), bufferSize);
   }

   return true;
}

void FMVoice::Start(double time, float target)
{
   if (mHarm.GetADSR()->GetR() <= 1)
      mHarm.GetADSR()->Clear();
   if (mModIdx.GetR() <= 1)
      mModIdx.Clear();
   if (mHarm2.GetADSR()->GetR() <= 1)
      mHarm2.GetADSR()->Clear();
   if (mModIdx2.GetR() <= 1)
      mModIdx2.Clear();

   mOsc.Start(time, target,
              mVoiceParams->mOscADSRParams);
   mHarm.Start(time, 1,
               mVoiceParams->mHarmRatioADSRParams);
   mModIdx.Start(time, 1,
                 mVoiceParams->mModIdxADSRParams);
   mHarm2.Start(time, 1,
                mVoiceParams->mHarmRatioADSRParams2);
   mModIdx2.Start(time, 1,
                  mVoiceParams->mModIdxADSRParams2);
}

void FMVoice::Stop(double time)
{
   mOsc.Stop(time);
   if (mHarm.GetADSR()->GetR() > 1)
      mHarm.Stop(time);
   if (mModIdx.GetR() > 1)
      mModIdx.Stop(time);
   if (mHarm2.GetADSR()->GetR() > 1)
      mHarm2.Stop(time);
   if (mModIdx2.GetR() > 1)
      mModIdx2.Stop(time);
}

void FMVoice::ClearVoice()
{
   mOsc.GetADSR()->Clear();
   mHarm.GetADSR()->Clear();
   mModIdx.Clear();
   mHarm2.GetADSR()->Clear();
   mModIdx2.Clear();
   mOscPhase = 0;
   mHarmPhase = 0;
   mHarmPhase2 = 0;
}

void FMVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<FMVoiceParams*>(params);
}

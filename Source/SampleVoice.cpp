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

SampleVoice::SampleVoice(IDrawableModule* owner)
: mPos(0)
, mOwner(owner)
{
}

SampleVoice::~SampleVoice()
{
}

bool SampleVoice::IsDone(double time)
{
   return mAdsr.IsDone(time);
}

void SampleVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("SampleVoice");

   if (IsDone(time) ||
       mVoiceParams->mSampleData == nullptr ||
       mVoiceParams->mSampleLength == 0)
      return;
   
   float volSq = mVoiceParams->mVol * mVoiceParams->mVol;
   
   for (int pos=0; pos<bufferSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      float freq = TheScale->PitchToFreq(GetPitch(pos));
      float speed;
      if (mVoiceParams->mDetectedFreq != -1)
         speed = freq/mVoiceParams->mDetectedFreq;
      else
         speed = freq/TheScale->PitchToFreq(TheScale->ScaleRoot()+48);
      
      out[pos] += GetInterpolatedSample(mPos, mVoiceParams->mSampleData, mVoiceParams->mSampleLength) * mAdsr.Value(time) * volSq;
      
      mPos += speed;
      
      time += gInvSampleRateMs;
   }
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

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

bool SampleVoice::Process(double time, ChannelBuffer* out)
{
   PROFILER(SampleVoice);

   if (IsDone(time) ||
       mVoiceParams->mSampleData == nullptr ||
       mVoiceParams->mSampleLength == 0)
      return false;
   
   float volSq = mVoiceParams->mVol * mVoiceParams->mVol;
   
   for (int pos=0; pos<out->BufferSize(); ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      if (mPos <= mVoiceParams->mSampleLength || mVoiceParams->mLoop)
      {
         float freq = TheScale->PitchToFreq(GetPitch(pos));
         float speed;
         if (mVoiceParams->mDetectedFreq != -1)
            speed = freq/mVoiceParams->mDetectedFreq;
         else
            speed = freq/TheScale->PitchToFreq(TheScale->ScaleRoot()+48);
         
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

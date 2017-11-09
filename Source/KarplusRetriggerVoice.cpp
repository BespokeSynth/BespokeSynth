//
//  KarplusRetriggerVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/18/14.
//
//

#include "KarplusRetriggerVoice.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"

KarplusRetriggerVoice::KarplusRetriggerVoice(IDrawableModule* owner)
: mStartTime(-1)
, mRetriggerTimer(0)
, mOwner(owner)
, mKarplusVoice(owner)
{
   mKarplusVoiceParams.mFilter = .6f;
   mKarplusVoiceParams.mVol = .1f;
   mKarplusVoiceParams.mFeedback = .998f;
   mKarplusVoiceParams.mSourceType = kSourceTypeNoise;
   mKarplusVoiceParams.mMute = true;
   mKarplusVoiceParams.mStretch = false;
   mKarplusVoiceParams.mCarrier = 100;
   mKarplusVoiceParams.mExcitation = 0;
   
   mKarplusVoice.SetVoiceParams(&mKarplusVoiceParams);
}

KarplusRetriggerVoice::~KarplusRetriggerVoice()
{
}

bool KarplusRetriggerVoice::IsDone(double time)
{
   return true;
}

void KarplusRetriggerVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("KarplusRetriggerVoice");

   if (mStartTime == -1)
      return;
   
   mKarplusVoice.SetPitch(GetPitch(0));
   
   mRetriggerTimer += bufferSize * gInvSampleRateMs;
   float timeBetween = MAX(mVoiceParams->mTimeBetween, 3);
   if (mRetriggerTimer * ofRandom(1.5f) >= timeBetween && mVoiceParams->mVelocity > 0)
   {
      mKarplusVoice.Start(time, mVoiceParams->mVelocity);
      mRetriggerTimer = 0;
   }
   
   mKarplusVoiceParams.mFilter = mVoiceParams->mFilter;
   mKarplusVoiceParams.mFeedback = mVoiceParams->mFeedback;
   
   mKarplusVoice.Process(time, out, bufferSize);
}

void KarplusRetriggerVoice::Start(double time, float target)
{
   mStartTime = time;
}

void KarplusRetriggerVoice::Stop(double time)
{
}

void KarplusRetriggerVoice::ClearVoice()
{
   mStartTime = -1;
}

void KarplusRetriggerVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<KarplusRetriggerVoiceParams*>(params);
}

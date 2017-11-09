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

FMVoice::FMVoice(IDrawableModule* owner)
: mOscPhase(0)
, mHarmPhase(0)
, mOsc(kOsc_Sin)
, mHarm(kOsc_Sin)
, mOwner(owner)
{
}

FMVoice::~FMVoice()
{
}

bool FMVoice::IsDone(double time)
{
   return mOsc.GetADSR()->IsDone(time);
}

void FMVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("FMVoice");

   if (IsDone(time))
      return;

   for (int pos=0; pos<bufferSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      float oscFreq = TheScale->PitchToFreq(GetPitch(pos));
      
      float harmFreq = oscFreq * mHarm.GetADSR()->Value(time) * mVoiceParams->mHarmRatio;
      float harmPhaseInc = GetPhaseInc(harmFreq);
      
      mHarmPhase += harmPhaseInc;
      while (mHarmPhase > FTWO_PI) { mHarmPhase -= FTWO_PI; }

      float modOscFreq = oscFreq + mHarm.Audio(time, mHarmPhase) * harmFreq * mModIdx.Value(time) * (mVoiceParams->mModIdx + GetModWheel(pos)*4) / mVoiceParams->mHarmRatio;
      float oscPhaseInc = GetPhaseInc(modOscFreq);

      mOscPhase += oscPhaseInc;
      while (mOscPhase > FTWO_PI) { mOscPhase -= FTWO_PI; }

      out[pos] += mOsc.Audio(time, mOscPhase) * mVoiceParams->mVol/20.0f;

      time += gInvSampleRateMs;
   }
}

void FMVoice::Start(double time, float target)
{
   mOsc.Start(time, target,
              mVoiceParams->mOscADSRParams);
   mHarm.Start(time, 1,
               mVoiceParams->mHarmRatioADSRParams);
   mModIdx.Start(time, 1,
                 mVoiceParams->mModIdxADSRParams);
}

void FMVoice::Stop(double time)
{
   mOsc.Stop(time);
   if (mHarm.GetADSR()->GetR() > 1)
      mHarm.Stop(time);
   if (mModIdx.GetR() > 1)
      mModIdx.Stop(time);
}

void FMVoice::ClearVoice()
{
   mOsc.GetADSR()->Clear();
   mHarm.GetADSR()->Clear();
   mModIdx.Clear();
}

void FMVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<FMVoiceParams*>(params);
}

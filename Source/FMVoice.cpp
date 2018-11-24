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

FMVoice::FMVoice(IDrawableModule* owner)
: mOscPhase(0)
, mHarmPhase(0)
, mHarmPhase2(0)
, mOsc(kOsc_Sin)
, mHarm(kOsc_Sin)
, mHarm2(kOsc_Sin)
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

bool FMVoice::Process(double time, ChannelBuffer* out)
{
   PROFILER(FMVoice);

   if (IsDone(time))
      return false;

   for (int pos=0; pos<out->BufferSize(); ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      float oscFreq = TheScale->PitchToFreq(GetPitch(pos));
      float harmFreq = oscFreq * mHarm.GetADSR()->Value(time) * mVoiceParams->mHarmRatio;
      float harmFreq2 = harmFreq * mHarm2.GetADSR()->Value(time) * mVoiceParams->mHarmRatio2;
      
      float harmPhaseInc2 = GetPhaseInc(harmFreq2);
      
      mHarmPhase2 += harmPhaseInc2;
      while (mHarmPhase2 > FTWO_PI) { mHarmPhase2 -= FTWO_PI; }
      
      float modHarmFreq = harmFreq + mHarm2.Audio(time, mHarmPhase2 + mVoiceParams->mPhaseOffset2) * harmFreq2 * mModIdx2.Value(time) * mVoiceParams->mModIdx2;
      
      float harmPhaseInc = GetPhaseInc(modHarmFreq);
      
      mHarmPhase += harmPhaseInc;
      while (mHarmPhase > FTWO_PI) { mHarmPhase -= FTWO_PI; }

      float modOscFreq = oscFreq + mHarm.Audio(time, mHarmPhase + mVoiceParams->mPhaseOffset1) * harmFreq * mModIdx.Value(time) * mVoiceParams->mModIdx;
      float oscPhaseInc = GetPhaseInc(modOscFreq);

      mOscPhase += oscPhaseInc;
      while (mOscPhase > FTWO_PI) { mOscPhase -= FTWO_PI; }

      float sample = mOsc.Audio(time, mOscPhase + mVoiceParams->mPhaseOffset0) * mVoiceParams->mVol/20.0f;
      if (out->NumActiveChannels() == 1)
      {
         out->GetChannel(0)[pos] += sample;
      }
      else
      {
         out->GetChannel(0)[pos] += sample * GetLeftPanGain(GetPan());
         out->GetChannel(1)[pos] += sample * GetRightPanGain(GetPan());
      }

      time += gInvSampleRateMs;
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

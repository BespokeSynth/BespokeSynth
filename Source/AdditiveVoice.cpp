//
//  AdditiveVoice.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#include "AdditiveVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"

AdditiveVoice::AdditiveVoice(IDrawableModule* owner)
   : mPhase(0)
   , mSyncPhase(0)
   , mOwner(owner)
{
   mOscs.push_back(new EnvOscillator(kOsc_Saw));
   mOscs.push_back(new EnvOscillator(kOsc_Sin));
   mOscs.push_back(new EnvOscillator(kOsc_Square));
   mOscs.push_back(new EnvOscillator(kOsc_Tri));
}

AdditiveVoice::~AdditiveVoice()
{
   for (int i=0; i<mOscs.size(); ++i)
      delete mOscs[i];
}

bool AdditiveVoice::IsDone(double time)
{
   for (int i=0; i<mOscs.size(); ++i)
   {
      if (mOscs[i]->GetADSR()->IsDone(time) == false)
         return false;
   }
   return true;
}

bool AdditiveVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("AdditiveVoice");

   assert(mOscs.size() == 4);
   
   if (IsDone(time))
      return false;
   
   for (int i=0; i<mOscs.size(); ++i)
      mOscs[i]->SetPulseWidth(mVoiceParams->mPulseWidth);
   
   float syncPhaseInc = GetPhaseInc(mVoiceParams->mSyncFreq);
   for (int pos=0; pos<bufferSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      float freq = TheScale->PitchToFreq(GetPitch(pos));
      float phaseInc = GetPhaseInc(freq);
      
      mPhase += phaseInc;
      while (mPhase > FTWO_PI*2)
      {
         mPhase -= FTWO_PI*2;
         mSyncPhase = 0;
      }
      mSyncPhase += syncPhaseInc;
      
      if (mVoiceParams->mSync)
      {
         for (int i=0; i<mOscs.size(); ++i)
            out[pos] += mOscs[i]->Audio(time, mSyncPhase*MAX(.5f,mVoiceParams->mData[i].mOctave)) * mVoiceParams->mData[i].mVol * mVoiceParams->mVol;
      }
      else
      {
         for (int i=0; i<mOscs.size(); ++i)
            out[pos] += mOscs[i]->Audio(time, mPhase*MAX(.5f,mVoiceParams->mData[i].mOctave)) * mVoiceParams->mData[i].mVol * mVoiceParams->mVol;
      }
      
      out[pos] += RandomSample() * mVoiceParams->mData[4].mVol * mNoiseAdsr.Value(time) * mVoiceParams->mVol;
      time += gInvSampleRateMs;
   }
   
   return true;
}

void AdditiveVoice::Start(double time, float target)
{
   assert(mOscs.size() == 4);
   for (int i=0; i<mOscs.size(); ++i)
   {
      mOscs[i]->Start(time, target,
                      mVoiceParams->mData[i].mAdsr);
   }
   mNoiseAdsr.Start(time, target,
                    mVoiceParams->mData[4].mAdsr);
}

void AdditiveVoice::Stop(double time)
{
   for (int i=0; i<mOscs.size(); ++i)
      mOscs[i]->Stop(time);
   mNoiseAdsr.Stop(time);
}

void AdditiveVoice::ClearVoice()
{
   for (int i=0; i<mOscs.size(); ++i)
      mOscs[i]->GetADSR()->Clear();
   mNoiseAdsr.Clear();
}

void AdditiveVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<AdditiveVoiceParams*>(params);
}

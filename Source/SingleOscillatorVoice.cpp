//
//  SingleOscillatorVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#include "SingleOscillatorVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"

SingleOscillatorVoice::SingleOscillatorVoice(IDrawableModule* owner)
: mPhase(0)
, mSyncPhase(0)
, mOsc(kOsc_Square)
, mStartTime(-1)
, mUseFilter(false)
, mOwner(owner)
{
}

SingleOscillatorVoice::~SingleOscillatorVoice()
{
}

bool SingleOscillatorVoice::IsDone(double time)
{
   return mOsc.GetADSR()->IsDone(time);
}

void SingleOscillatorVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("SingleOscillatorVoice");

   if (IsDone(time))
      return;
   
   mOsc.SetType(mVoiceParams->mOscType);
   
   float syncPhaseInc = GetPhaseInc(mVoiceParams->mSyncFreq);
   for (int pos=0; pos<bufferSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      mOsc.SetPulseWidth(mVoiceParams->mPulseWidth);
      mOsc.mOsc.SetShuffle(mVoiceParams->mShuffle);
      
      float pitch = GetPitch(pos);
      float freq = TheScale->PitchToFreq(pitch) * mVoiceParams->mMult;
      
      float phaseInc = GetPhaseInc(freq * mVoiceParams->mDetune);
      
      mPhase += phaseInc;
      if (mPhase == INFINITY)
      {
         ofLog() << "Infinite phase. phaseInc:" + ofToString(phaseInc) + " detune:" + ofToString(mVoiceParams->mDetune) + " freq:" + ofToString(freq) + " pitch:" + ofToString(pitch) + " getpitch:" + ofToString(GetPitch(pos));
      }
      while (mPhase > FTWO_PI*2)
      {
         mPhase -= FTWO_PI*2;
         mSyncPhase = 0;
      }
      mSyncPhase += syncPhaseInc;
      
      float sample;
      float vol = mVoiceParams->mVol * .1f;
      
      if (mVoiceParams->mPressureEnvelope)
         vol *= GetPressure(pos);
      
      if (mVoiceParams->mSync)
         sample = mOsc.Audio(time, mSyncPhase) * vol;
      else
         sample = mOsc.Audio(time, mPhase + mVoiceParams->mPhaseOffset) * vol;
      
      if (mUseFilter)
      {
         float f = mFilterAdsr.Value(time) * mVoiceParams->mFilterCutoff;
         float q = 1;
         mFilter.SetFilterParams(f, q);
         sample = mFilter.Filter(sample);
      }
      
      out[pos] += sample;
      
      time += gInvSampleRateMs;
   }
}

void SingleOscillatorVoice::Start(double time, float target)
{
   mOsc.Start(time, mVoiceParams->mPressureEnvelope ? 1 : target, mVoiceParams->mAdsr);
   mStartTime = time;
   
   if (mVoiceParams->mFilterCutoff != SINGLEOSCILLATOR_NO_CUTOFF ||
       mVoiceParams->mFilterAdsr.GetA() > 1 ||
       mVoiceParams->mFilterAdsr.GetS() < 1 ||
       mVoiceParams->mFilterAdsr.GetR() > 30)
   {
      mUseFilter = true;
      mFilter.SetFilterType(kFilterType_Lowpass);
      mFilterAdsr = mVoiceParams->mFilterAdsr;
      mFilterAdsr.Start(time,1);
   }
   else
   {
      mUseFilter = false;
   }
}

void SingleOscillatorVoice::Stop(double time)
{
   mOsc.Stop(time);
}

void SingleOscillatorVoice::ClearVoice()
{
   mOsc.GetADSR()->Clear();
   mFilterAdsr.Clear();
   mPhase = 0;
   mSyncPhase = 0;
}

void SingleOscillatorVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<OscillatorVoiceParams*>(params);
}

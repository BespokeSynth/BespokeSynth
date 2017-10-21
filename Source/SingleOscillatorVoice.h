//
//  SingleOscillatorVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#ifndef __modularSynth__SingleOscillatorVoice__
#define __modularSynth__SingleOscillatorVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"
#include "LFO.h"
#include "BiquadFilter.h"

#define SINGLEOSCILLATOR_NO_CUTOFF 10000

class OscillatorVoiceParams : public IVoiceParams
{
public:
   ADSR mAdsr;
   float mVol;
   float mPulseWidth;
   bool mSync;
   float mSyncFreq;
   float mMult;
   OscillatorType mOscType;
   float mDetune;
   float mShuffle;
   
   float mFilterCutoff;
   ADSR mFilterAdsr;
   
   bool mPressureEnvelope;
};

class SingleOscillatorVoice : public IMidiVoice
{
public:
   SingleOscillatorVoice(IDrawableModule* owner = nullptr);
   ~SingleOscillatorVoice();
   
   // IMidiVoice
   void Start(double time, float amount);
   void Stop(double time);
   void ClearVoice();
   void Process(double time, float* out, int bufferSize);
   void SetVoiceParams(IVoiceParams* params);
private:
   float mPhase;
   float mSyncPhase;
   EnvOscillator mOsc;
   OscillatorVoiceParams* mVoiceParams;
   double mStartTime;
   
   ADSR mFilterAdsr;
   BiquadFilter mFilter;
   bool mUseFilter;
   
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__SingleOscillatorVoice__) */

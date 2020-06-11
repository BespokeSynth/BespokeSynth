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
   ::ADSR mAdsr;
   float mVol;
   float mPulseWidth;
   bool mSync;
   float mSyncFreq;
   float mMult;
   OscillatorType mOscType;
   float mDetune;
   float mShuffle;
   float mPhaseOffset;
   int mUnison;
   float mUnisonWidth;
   
   float mFilterCutoff;
   float mFilterQ;
   ::ADSR mFilterAdsr;
};

class SingleOscillatorVoice : public IMidiVoice
{
public:
   SingleOscillatorVoice(IDrawableModule* owner = nullptr);
   ~SingleOscillatorVoice();
   
   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
   
   static const int kMaxUnison = 8;
private:
   struct OscData
   {
      OscData() : mPhase(0), mSyncPhase(0), mOsc(kOsc_Square), mDetuneFactor(0) {}
      float mPhase;
      float mSyncPhase;
      Oscillator mOsc;
      float mDetuneFactor;
   };
   OscData mOscData[kMaxUnison];
   ::ADSR mAdsr;
   OscillatorVoiceParams* mVoiceParams;
   
   ::ADSR mFilterAdsr;
   BiquadFilter mFilterLeft;
   BiquadFilter mFilterRight;
   bool mUseFilter;
   
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__SingleOscillatorVoice__) */

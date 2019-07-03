//
//  FMVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#ifndef __modularSynth__FMVoice__
#define __modularSynth__FMVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class FMVoiceParams : public IVoiceParams
{
public:
   ::ADSR mOscADSRParams;
   ::ADSR mModIdxADSRParams;
   ::ADSR mHarmRatioADSRParams;
   ::ADSR mModIdxADSRParams2;
   ::ADSR mHarmRatioADSRParams2;
   float mModIdx;
   float mHarmRatio;
   float mModIdx2;
   float mHarmRatio2;
   float mVol;
   float mPhaseOffset0;
   float mPhaseOffset1;
   float mPhaseOffset2;
};

class FMVoice : public IMidiVoice
{
public:
   FMVoice(IDrawableModule* owner = nullptr);
   ~FMVoice();

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
private:
   float mOscPhase;
   EnvOscillator mOsc;
   float mHarmPhase;
   EnvOscillator mHarm;
   ::ADSR mModIdx;
   float mHarmPhase2;
   EnvOscillator mHarm2;
   ::ADSR mModIdx2;
   FMVoiceParams* mVoiceParams;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__FMVoice__) */

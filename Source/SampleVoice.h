//
//  SampleVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#ifndef __modularSynth__SampleVoice__
#define __modularSynth__SampleVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class SampleVoiceParams : public IVoiceParams
{
public:
   ::ADSR mAdsr;
   float mVol;
   float* mSampleData;
   int mSampleLength;
   float mDetectedFreq;
   bool mLoop;
};

class SampleVoice : public IMidiVoice
{
public:
   SampleVoice(IDrawableModule* owner = nullptr);
   ~SampleVoice();
   
   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
private:
   ::ADSR mAdsr;
   SampleVoiceParams* mVoiceParams;
   float mPos;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__SampleVoice__) */

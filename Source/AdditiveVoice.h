//
//  AdditiveVoice.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#ifndef __additiveSynth__AdditiveVoice__
#define __additiveSynth__AdditiveVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"

class EnvOscillator;
class IDrawableModule;

struct AdditiveVoiceData
{
   ADSR mAdsr;
   float mVol;
   int mOctave;
};

class AdditiveVoiceParams : public IVoiceParams
{
public:
   AdditiveVoiceData mData[5];
   float mVol;
   float mPulseWidth;
   bool mSync;
   float mSyncFreq;
};

class AdditiveVoice : public IMidiVoice
{
public:
   AdditiveVoice(IDrawableModule* owner = nullptr);
   ~AdditiveVoice();
   
   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   void Process(double time, float* out, int bufferSize) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
private:
   float mPhase;
   float mSyncPhase;
   std::vector<EnvOscillator*> mOscs;
   ADSR mNoiseAdsr;
   AdditiveVoiceParams* mVoiceParams;
   IDrawableModule* mOwner;
};

#endif /* defined(__additiveSynth__AdditiveVoice__) */

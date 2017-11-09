//
//  KarplusStrongVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#ifndef __modularSynth__KarplusStrongVoice__
#define __modularSynth__KarplusStrongVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"
#include "RollingBuffer.h"
#include "Ramp.h"

class IDrawableModule;

enum KarplusStrongSourceType
{
   kSourceTypeSin,
   kSourceTypeNoise,
   kSourceTypeMix,
   kSourceTypeSaw
};

class KarplusStrongVoiceParams : public IVoiceParams
{
public:
   float mFilter;
   float mVol;
   float mFeedback;
   KarplusStrongSourceType mSourceType;
   bool mMute;
   bool mStretch;
   float mCarrier;
   float mExcitation;
};

class KarplusStrongVoice : public IMidiVoice
{
public:
   KarplusStrongVoice(IDrawableModule* owner = nullptr);
   ~KarplusStrongVoice();

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   void Process(double time, float* out, int bufferSize) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
private:
   float mOscPhase;
   EnvOscillator mOsc;
   ADSR mEnv;
   KarplusStrongVoiceParams* mVoiceParams;
   RollingBuffer mBuffer;
   float mFilterSample;
   Ramp mMuteRamp;
   float mLastBufferSample;
   bool mActive;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__KarplusStrongVoice__) */

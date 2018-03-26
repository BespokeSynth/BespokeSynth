//
//  KarplusRetriggerVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/18/14.
//
//

#ifndef __modularSynth__KarplusRetriggerVoice__
#define __modularSynth__KarplusRetriggerVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "KarplusStrongVoice.h"

class IDrawableModule;

class KarplusRetriggerVoiceParams : public IVoiceParams
{
public:
   KarplusRetriggerVoiceParams() :
   mVelocity(0),
   mTimeBetween(1000),
   mFilter(.6f),
   mFeedback(.998f)
   {}
   
   float mVelocity;
   float mTimeBetween;
   float mFilter;
   float mFeedback;
};

class KarplusRetriggerVoice : public IMidiVoice
{
public:
   KarplusRetriggerVoice(IDrawableModule* owner = nullptr);
   ~KarplusRetriggerVoice();
   
   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, float* out, int bufferSize) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;
private:
   KarplusStrongVoice mKarplusVoice;
   KarplusStrongVoiceParams mKarplusVoiceParams;
   KarplusRetriggerVoiceParams* mVoiceParams;
   
   double mStartTime;
   float mRetriggerTimer;
   
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__KarplusRetriggerVoice__) */

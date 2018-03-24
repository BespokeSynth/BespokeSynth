//
//  PolyphonyMgr.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#ifndef __additiveSynth__PolyphonyMgr__
#define __additiveSynth__PolyphonyMgr__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"

const int kVoiceFadeSamples = 50;

class IMidiVoice;
class IVoiceParams;
class IDrawableModule;
struct ModulationParameters;

enum VoiceType
{
   kVoiceType_Additive,
   kVoiceType_Karplus,
   kVoiceType_FM,
   kVoiceType_SingleOscillator,
   kVoiceType_Sampler
};

struct PitchAndVoice
{
   PitchAndVoice() : mPitch(-1) {}
   
   float mPitch;
   IMidiVoice* mVoice;
   double mTime;
};

class PolyphonyMgr
{
public:
   PolyphonyMgr(IDrawableModule* owner);
   ~PolyphonyMgr();
   
   void Init(VoiceType type,
             IVoiceParams* mVoiceParams);
   
   void Start(double time, int pitch, float amount, int voiceIdx, ModulationParameters modulation);
   void Stop(double time, int pitch);
   void Process(double time, float* out, int bufferSize);
   void GetPhaseAndInc(float& phase, float& inc);
private:
   void Prune(double time);
   
   PitchAndVoice mVoices[kNumVoices];
   bool mAllowStealing;
   int mLastVoice;
   float mFadeOutWriteBuffer[kVoiceFadeSamples];
   float mFadeOutBuffer[kVoiceFadeSamples];
   int mFadeOutBufferPos;
   IDrawableModule* mOwner;
};

#endif /* defined(__additiveSynth__PolyphonyMgr__) */

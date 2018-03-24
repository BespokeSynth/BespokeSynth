//
//  IMidiVoice.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#ifndef additiveSynth_IMidiVoice_h
#define additiveSynth_IMidiVoice_h

#include "ModulationChain.h"
#include "Profiler.h"

class IVoiceParams;

class IMidiVoice
{
public:
   IMidiVoice() : mPitch(0) {}
   virtual ~IMidiVoice() {}
   virtual void ClearVoice() = 0;
   void SetPitch(float pitch) { mPitch = pitch; }
   void SetModulators(ModulationParameters modulators) { mModulators = modulators; }
   virtual void Start(double time, float amount) = 0;
   virtual void Stop(double time) = 0;
   virtual void Process(double time, float* out, int bufferSize) = 0;
   virtual bool IsDone(double time) = 0;
   virtual void SetVoiceParams(IVoiceParams* params) = 0;
   
   float GetPitch(int samplesIn) { return mPitch + (mModulators.pitchBend ? mModulators.pitchBend->GetValue(samplesIn) : 0); }
   float GetModWheel(int samplesIn) { return mModulators.modWheel ? mModulators.modWheel->GetValue(samplesIn) : 0; }
   float GetPressure(int samplesIn) { return mModulators.pressure ? mModulators.pressure->GetValue(samplesIn) : 0; }
private:
   float mPitch;
   ModulationParameters mModulators;
};

#endif

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
   IMidiVoice() : mPitch(0), mPitchBend(NULL), mModWheel(NULL), mPressure(NULL) {}
   virtual ~IMidiVoice() {}
   virtual void ClearVoice() = 0;
   void SetPitch(float pitch) { mPitch = pitch; }
   void SetModulators(ModulationChain* pitchBend, ModulationChain* modWheel, ModulationChain* pressure) { mPitchBend = pitchBend; mModWheel = modWheel; mPressure = pressure; }
   virtual void Start(double time, float amount) = 0;
   virtual void Stop(double time) = 0;
   virtual void Process(double time, float* out, int bufferSize) = 0;
   virtual void SetVoiceParams(IVoiceParams* params) = 0;
   
   float GetPitch(int samplesIn) { return mPitch + (mPitchBend ? mPitchBend->GetValue(samplesIn) : 0); }
   float GetModWheel(int samplesIn) { return mModWheel ? mModWheel->GetValue(samplesIn) : 0; }
   float GetPressure(int samplesIn) { return mPressure ? mPressure->GetValue(samplesIn) : 0; }
private:
   float mPitch;
   ModulationChain* mPitchBend;
   ModulationChain* mModWheel;
   ModulationChain* mPressure;
};

#endif

//
//  ModulationChain.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/15.
//
//

#ifndef __Bespoke__ModulationChain__
#define __Bespoke__ModulationChain__

#include "Ramp.h"
#include "LFO.h"

class ModulationChain
{
public:
   ModulationChain();
   float GetValue(int samplesIn) const;
   float GetIndividualValue(int samplesIn) const;
   void SetValue(float value);
   void RampValue(float from, float to, double time);
   void SetLFO(NoteInterval interval, float amount);
   void AppendTo(ModulationChain* chain);
   void SetSidechain(ModulationChain* chain);
   void MultiplyIn(ModulationChain* chain);
private:
   Ramp mRamp;
   LFO mLFO;
   float mLFOAmount;
   ModulationChain* mPrev;
   ModulationChain* mSidechain;
   ModulationChain* mMultiplyIn;
};

struct ModulationCollection
{
   ModulationChain mPitchBend;
   ModulationChain mModWheel;
   ModulationChain mPressure;
};

struct ModulationParameters
{
   ModulationParameters() {}
   ModulationParameters(ModulationChain* _pitchBend,
                        ModulationChain* _modWheel,
                        ModulationChain* _pressure,
                        float _pan) : pitchBend(_pitchBend), modWheel(_modWheel), pressure(_pressure), pan(_pan) {}
   ModulationChain* pitchBend = nullptr;
   ModulationChain* modWheel = nullptr;
   ModulationChain* pressure = nullptr;
   float pan = 0;
};

class Modulations
{
public:
   Modulations(bool isGlobalEffect);   //isGlobalEffect: is the effect that we're using this on a global effect that affects all voices (pitch bend all voices that come through here the same way) or a voice effect (affect individual voices that come through here individually)?
   ModulationChain* GetPitchBend(int voiceIdx);
   ModulationChain* GetModWheel(int voiceIdx);
   ModulationChain* GetPressure(int voiceIdx);
private:
   ModulationCollection mGlobalModulation;
   vector<ModulationCollection> mVoiceModulations;
};

#endif /* defined(__Bespoke__ModulationChain__) */

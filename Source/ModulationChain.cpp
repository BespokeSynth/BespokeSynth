//
//  ModulationChain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/15.
//
//

#include "ModulationChain.h"

ModulationChain::ModulationChain()
: mLFOAmount(0)
, mPrev(nullptr)
, mSidechain(nullptr)
, mMultiplyIn(nullptr)
{
   mLFO.SetMode(kLFOMode_Oscillator);
}

float ModulationChain::GetValue(int samplesIn) const
{
   float value = GetIndividualValue(samplesIn);
   if (mMultiplyIn)
      value *= mMultiplyIn->GetIndividualValue(samplesIn);
   if (mSidechain)
      value += mSidechain->GetIndividualValue(samplesIn);
   if (mPrev)
      value += mPrev->GetValue(samplesIn);
   if (value != value)
      return 0;
   return value;
}

float ModulationChain::GetIndividualValue(int samplesIn) const
{
   double time = gTime + gInvSampleRateMs*samplesIn;
   float value = mRamp.Value(time);
   if (mLFOAmount != 0)
      value += mLFO.Value(samplesIn) * mLFOAmount;
   return value;
}

void ModulationChain::SetValue(float value)
{
   mRamp.Start(value, gInvSampleRateMs*gBufferSize);
}

void ModulationChain::RampValue(float from, float to, double time)
{
   mRamp.Start(gTime, from, to, gTime + time);
}

void ModulationChain::SetLFO(NoteInterval interval, float amount)
{
   mLFO.SetPeriod(interval);
   mLFOAmount = amount;
}

void ModulationChain::AppendTo(ModulationChain* chain)
{
   mPrev = chain;
}

void ModulationChain::SetSidechain(ModulationChain* chain)
{
   mSidechain = chain;
}

void ModulationChain::MultiplyIn(ModulationChain* chain)
{
   mMultiplyIn = chain;
}

Modulations::Modulations(bool isGlobalEffect)
{
   mVoiceModulations.resize(kNumVoices);
   
   if (isGlobalEffect)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoiceModulations[i].mPitchBend.SetSidechain(&mGlobalModulation.mPitchBend);
         mVoiceModulations[i].mModWheel.SetSidechain(&mGlobalModulation.mModWheel);
         mVoiceModulations[i].mPressure.SetSidechain(&mGlobalModulation.mPressure);
      }
   }
}

ModulationChain* Modulations::GetPitchBend(int voiceIdx)
{
   if (voiceIdx == -1)
      return &mGlobalModulation.mPitchBend;
   else
      return &mVoiceModulations[voiceIdx].mPitchBend;
}

ModulationChain* Modulations::GetModWheel(int voiceIdx)
{
   if (voiceIdx == -1)
      return &mGlobalModulation.mModWheel;
   else
      return &mVoiceModulations[voiceIdx].mModWheel;
}

ModulationChain* Modulations::GetPressure(int voiceIdx)
{
   if (voiceIdx == -1)
      return &mGlobalModulation.mPressure;
   else
      return &mVoiceModulations[voiceIdx].mPressure;
}

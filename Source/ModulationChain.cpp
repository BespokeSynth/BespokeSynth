/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  ModulationChain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/15.
//
//

#include "ModulationChain.h"

ModulationChain::ModulationChain(float initialValue)
{
   mRamp.SetValue(initialValue);
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
   if (std::isnan(value))
      return 0;
   return value;
}

float ModulationChain::GetIndividualValue(int samplesIn) const
{
   double time = gTime + gInvSampleRateMs * samplesIn;
   float value;
   if (mRamp.HasValue(time))
      value = mRamp.Value(time);
   else
      value = 0;
   if (mLFOAmount != 0)
      value += mLFO.Value(samplesIn) * mLFOAmount;
   if (mBuffer != nullptr && samplesIn >= 0 && samplesIn < gBufferSize)
      value += mBuffer[samplesIn];
   return value;
}

void ModulationChain::SetValue(float value)
{
   mRamp.Start(gTime, value, gTime + gInvSampleRateMs * gBufferSize);
}

void ModulationChain::RampValue(double time, float from, float to, double length)
{
   mRamp.Start(time, from, to, time + length);
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

void ModulationChain::CreateBuffer()
{
   if (mBuffer == nullptr)
      mBuffer = new float[gBufferSize];
   Clear(mBuffer, gBufferSize);
}

void ModulationChain::FillBuffer(float* buffer)
{
   if (mBuffer != nullptr)
      BufferCopy(mBuffer, buffer, gBufferSize);
}

float ModulationChain::GetBufferValue(int sampleIdx)
{
   if (mBuffer != nullptr && sampleIdx >= 0 && sampleIdx < gBufferSize)
      return mBuffer[sampleIdx];
   return 0;
}

Modulations::Modulations(bool isGlobalEffect)
{
   mVoiceModulations.resize(kNumVoices);

   if (isGlobalEffect)
   {
      for (int i = 0; i < kNumVoices; ++i)
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

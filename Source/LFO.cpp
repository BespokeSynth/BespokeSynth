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
//  LFO.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#include "LFO.h"
#include "OpenFrameworksPort.h"
#include "Profiler.h"

//static
PerlinNoise LFO::sPerlinNoise;

LFO::LFO()
{
   SetPeriod(kInterval_1n);
}

LFO::~LFO()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

float LFO::CalculatePhase(int samplesIn /*= 0*/, bool doTransform /* = true*/) const
{
   float ret;
   if (mPeriod == kInterval_Free)
   {
      ret = mFreePhase + float(samplesIn) / gSampleRate * mFreeRate;
   }
   else
   {
      float period = TheTransport->GetDuration(mPeriod) / TheTransport->GetDuration(kInterval_1n);

      float phase = TheTransport->GetMeasureTime(gTime + samplesIn * gInvSampleRateMs) / period + (1 - mPhaseOffset - mAdjustOffset) + 10; //+10 so we can have negative samplesIn

      phase -= int(phase) / 2 * 2; //using 2 allows for shuffle to work

      ret = phase;
   }

   if (doTransform)
      return TransformPhase(ret);
   return ret;
}

float LFO::TransformPhase(float phase) const
{
   if (mLength != 1 && mLength > 0)
      phase = int(phase) + ofClamp((phase - int(phase)) / mLength, 0, 1);
   else if (mLength != 1)
      phase = 0;
   return phase;
}

float LFO::Value(int samplesIn /*= 0*/, float forcePhase /*= -1*/) const
{
   //PROFILER(LFO_Value);

   if (mPeriod == kInterval_None) //no oscillator
      return mMode == kLFOMode_Envelope ? 1 : 0;

   float phase = CalculatePhase(samplesIn);

   if (forcePhase != -1 && !std::isnan(forcePhase))
      phase = forcePhase;

   phase *= FTWO_PI;

   float sample;
   bool nonstandardOsc = false;

   //use sample-and-hold value
   if (mOsc.GetType() == kOsc_Random)
   {
      nonstandardOsc = true;
      sample = pow(mRandom.Value(gTime + samplesIn * gInvSampleRateMs), powf((1 - mOsc.GetPulseWidth()) * 2, 2));
   }
   else if (mOsc.GetType() == kOsc_Drunk)
   {
      nonstandardOsc = true;
      sample = mDrunk;
   }
   else if (mOsc.GetType() == kOsc_Perlin)
   {
      nonstandardOsc = true;

      double perlinPos = gTime + gInvSampleRateMs * samplesIn;
      if (forcePhase != -1 && !std::isnan(forcePhase))
         perlinPos += forcePhase * 1000;
      double perlinPhase = perlinPos * mFreeRate / 1000.0f;
      sample = sPerlinNoise.noise(perlinPhase, mPerlinSeed, -perlinPhase);
   }
   else
   {
      sample = mOsc.Value(phase);
      if (mMode == kLFOMode_Envelope) //rescale to 0 1
         sample = sample * .5f + .5f;
   }

   if (nonstandardOsc)
   {
      if (mOsc.GetPulseWidth() != .5f)
         sample = Bias(sample, mOsc.GetPulseWidth());

      if (mMode == kLFOMode_Oscillator) //rescale to -1 1
         sample = (sample - .5f * 2);
   }

   return sample;
}

void LFO::SetPeriod(NoteInterval interval)
{
   if (interval == kInterval_Free)
      mFreePhase = CalculatePhase();

   mPeriod = interval;
   if (mOsc.GetType() == kOsc_Random)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mPeriod;
   }

   if (mOsc.GetType() == kOsc_Drunk || mPeriod == kInterval_Free)
      TheTransport->AddAudioPoller(this);
   else
      TheTransport->RemoveAudioPoller(this);
}

void LFO::SetType(OscillatorType type)
{
   mOsc.SetType(type);

   if (type == kOsc_Random)
      TheTransport->AddListener(this, mPeriod, OffsetInfo(0, true), false);
   else
      TheTransport->RemoveListener(this);

   if (type == kOsc_Perlin)
      mPerlinSeed = gRandom() % 10000;

   if (mOsc.GetType() == kOsc_Drunk || mOsc.GetType() == kOsc_Perlin || mPeriod == kInterval_Free)
      TheTransport->AddAudioPoller(this);
   else
      TheTransport->RemoveAudioPoller(this);
}

void LFO::OnTimeEvent(double time)
{
   if (mOsc.GetSoften() == 0)
      mRandom.SetValue(ofRandom(1));
   else
      mRandom.Start(time, ofRandom(1), time + mOsc.GetSoften() * 30);
}

void LFO::OnTransportAdvanced(float amount)
{
   if (mOsc.GetType() == kOsc_Drunk)
   {
      float distance = 0;
      if (mPeriod == kInterval_Free)
      {
         distance = mFreeRate / 40;
      }
      else
      {
         distance = TheTransport->GetDuration(kInterval_64n) / TheTransport->GetDuration(mPeriod);
      }
      float drunk = ofRandom(-distance, distance);
      if (mDrunk + drunk > 1 || mDrunk + drunk < 0)
         drunk *= -1;
      mDrunk = ofClamp(mDrunk + drunk, 0, 1);
   }
   if (mPeriod == kInterval_Free || mOsc.GetType() == kOsc_Perlin)
   {
      mFreePhase += mFreeRate * amount * TheTransport->MsPerBar() / 1000;
      double wrap = mOsc.GetShuffle() > 0 ? 2 : 1;
      if (mFreePhase > wrap || mFreePhase < 0)
      {
         mFreePhase = fmod(mFreePhase, wrap);
         if (mOsc.GetType() == kOsc_Random)
            OnTimeEvent(gTime);
      }
   }
}

void LFO::ResetPhase(double time)
{
   mFreePhase = 1 - mPhaseOffset;
   mAdjustOffset = 0;
   mAdjustOffset = CalculatePhase() + mPhaseOffset;
}

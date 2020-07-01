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

LFO::LFO()
: mPhaseOffset(0)
, mOsc(kOsc_Sin)
, mPeriod(kInterval_1n)
, mDrunk(0)
, mMode(kLFOMode_Envelope)
, mFreePhase(0)
, mFreeRate(1)
, mLength(1)
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
      
      float phase = TheTransport->GetMeasureTime(gTime+samplesIn*gInvSampleRateMs) / period + (1 - mPhaseOffset) + 1;  //+1 so we can have negative samplesIn
      
      phase -= int(phase) / 2 * 2;  //using 2 allows for shuffle to work
      
      ret = phase;
   }
   
   if (doTransform)
      return TransformPhase(ret);
   return ret;
}

float LFO::TransformPhase(float phase) const
{
   if (mLength != 1)
      phase = int(phase) + ofClamp((phase - int(phase)) / mLength, 0, 1);
   return phase;
}

float LFO::Value(int samplesIn /*= 0*/, float forcePhase /*= -1*/) const
{
   //PROFILER(LFO_Value);
   
   if (mPeriod == kInterval_None)  //no oscillator
      return mMode == kLFOMode_Envelope ? 1 : 0;

   float phase = CalculatePhase(samplesIn);
   
   if (forcePhase != -1)
      phase = forcePhase;
   
   phase *= FTWO_PI;
   
   float sample;
   
   //use sample-and-hold value
   if (mOsc.GetType() == kOsc_Random &&
       !(mPeriod == kInterval_2 ||
         mPeriod == kInterval_3 ||
         mPeriod == kInterval_4 ||
         mPeriod == kInterval_8 ||
         mPeriod == kInterval_16 ||
         mPeriod == kInterval_32 ||
         mPeriod == kInterval_64))
   {
      sample = pow(mRandom.Value(gTime + samplesIn * gInvSampleRateMs), powf((1-mOsc.GetPulseWidth())*2, 2));
      if (mMode == kLFOMode_Oscillator)   //rescale to -1 1
         sample = (sample - .5f) * 2;
   }
   else if (mOsc.GetType() == kOsc_Drunk)
   {
      sample = mDrunk;
      if (mMode == kLFOMode_Oscillator)   //rescale to -1 1
         sample = (sample - .5f * 2);
   }
   else
   {
      sample = mOsc.Value(phase);
      if (mMode == kLFOMode_Envelope)     //rescale to 0 1
         sample = sample * .5f + .5f;
   }

   return sample;
}

void LFO::SetPeriod(NoteInterval interval)
{
   if (interval == kInterval_Free)
      mFreePhase = CalculatePhase();
   
   mPeriod = interval;
   if (mOsc.GetType() == kOsc_Random)
      TheTransport->UpdateListener(this, mPeriod);
   
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
   
   if (mOsc.GetType() == kOsc_Drunk || mPeriod == kInterval_Free)
      TheTransport->AddAudioPoller(this);
   else
      TheTransport->RemoveAudioPoller(this);
}

void LFO::OnTimeEvent(double time)
{
   if (mOsc.GetSoften() == 0)
      mRandom.SetValue(ofRandom(1));
   else
      mRandom.Start(ofRandom(1), mOsc.GetSoften() * 30);
}

void LFO::OnTransportAdvanced(float amount)
{
   if (mOsc.GetType() == kOsc_Drunk)
   {
      float distance = TheTransport->GetDuration(mPeriod) * .000005f;
      float drunk = ofRandom(-distance, distance);
      if (mDrunk + drunk > 1 || mDrunk + drunk < 0)
         drunk *= -1;
      mDrunk = ofClamp(mDrunk+drunk, 0, 1);
   }
   if (mPeriod == kInterval_Free)
   {
      mFreePhase += mFreeRate * amount * TheTransport->MsPerBar() / 1000;
      if (mFreePhase > 2)
         mFreePhase -= 2;
   }
}

//
//  LFO.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#include "LFO.h"
#include "OpenFrameworksPort.h"

LFO::LFO()
: mPhaseOffset(0)
, mOsc(kOsc_Sin)
, mPeriod(kInterval_1n)
, mRandom(0)
, mMode(kLFOMode_Envelope)
{
   SetPeriod(kInterval_1n);
}

LFO::~LFO()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

float LFO::Value(int samplesIn /*= 0*/) const
{
   if (mPeriod == kInterval_None)  //no oscillator
      return mMode == kLFOMode_Envelope ? 1 : 0;

   float period = TheTransport->GetDuration(mPeriod) / TheTransport->GetDuration(kInterval_1n);

   float sampsPerMeasure = TheTransport->MsPerBar() / gInvSampleRateMs;
   float phase = ((TheTransport->GetMeasurePos()+TheTransport->GetMeasure() + samplesIn/sampsPerMeasure) / period + mPhaseOffset + 1);  //+1 so we can have negative samplesIn

   phase -= int(phase);
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
      sample = pow(mRandom, powf((1-mOsc.GetPulseWidth())*2, 2));
      if (mMode == kLFOMode_Oscillator)   //rescale to -1 1
         sample = (sample - .5f) * 2;
   }
   else if (mOsc.GetType() == kOsc_Drunk)
   {
      sample = mRandom;
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
   mPeriod = interval;
   if (mOsc.GetType() == kOsc_Random)
      TheTransport->UpdateListener(this, mPeriod);
}

void LFO::SetType(OscillatorType type)
{
   mOsc.SetType(type);
   
   if (type == kOsc_Random)
      TheTransport->AddListener(this, mPeriod);
   else
      TheTransport->RemoveListener(this);
   
   if (type == kOsc_Drunk)
      TheTransport->AddAudioPoller(this);
   else
      TheTransport->RemoveAudioPoller(this);
}

void LFO::OnTimeEvent(int samplesTo)
{
   mRandom = ofRandom(1);
}

void LFO::OnTransportAdvanced(float amount)
{
   if (mOsc.GetType() == kOsc_Drunk)
   {
      float distance = TheTransport->GetDuration(mPeriod) * .000005f;
      float drunk = ofRandom(-distance, distance);
      if (mRandom + drunk > 1 || mRandom + drunk < 0)
         drunk *= -1;
      mRandom = ofClamp(mRandom+drunk, 0, 1);
   }
}

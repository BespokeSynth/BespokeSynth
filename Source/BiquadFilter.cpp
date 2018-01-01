//
//  BiquadFilter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/2/14.
//
//

#include "BiquadFilter.h"
#include "SynthGlobals.h"

BiquadFilter::BiquadFilter()
: mType(kFilterType_Lowpass)
, mF(4000)
, mQ(1)
, mDbGain(0)
{
   Clear();
   UpdateFilterCoeff();
}

void BiquadFilter::Clear()
{
   mHistPre1 = 0;
   mHistPre2 = 0;
   mHistPost1 = 0;
   mHistPost2 = 0;
}

void BiquadFilter::UpdateFilterCoeff()
{
   if (mF<=0)
   {
      mFF0 = 0;
      mFF1 = 0;
      mFF2 = 0;
      mFB1 = 0;
      mFB2 = 0;
      return;
   }
   
   float w0 = gTwoPiOverSampleRate*mF;
   float cosw0 = cos(w0);
   float sinw0 = sin(w0);
   float alpha = sinw0/(2*mQ);
   
   float b0 = 0;
   float b1 = 0;
   float b2 = 0;
   float a0 = 0;
   float a1 = 0;
   float a2 = 0;
   
   if (mType == kFilterType_Lowpass)
   {
      b0 = (1 - cosw0)/2;
      b1 =  1 - cosw0;
      b2 = (1 - cosw0)/2;
      a0 =  1 + alpha;
      a1 = -2 * cosw0;
      a2 =  1 - alpha;
   }
   
   if (mType == kFilterType_Highpass)
   {
      b0 = (1 + cosw0)/2;
      b1 =-(1 + cosw0);
      b2 = (1 + cosw0)/2;
      a0 =  1 + alpha;
      a1 = -2 * cosw0;
      a2 =  1 - alpha;
   }
   
   if (mType == kFilterType_Bandpass)
   {
      b0 = sinw0/2;
      b1 = 0;
      b2 = -sinw0/2;
      a0 =  1 + alpha;
      a1 = -2 * cosw0;
      a2 =  1 - alpha;
   }
   
   if (mType == kFilterType_PeakNotch)
   {
      float A = powf(10,(mDbGain/40));
      b0 =   1 + alpha*A;
      b1 =  -2*cosw0;
      b2 =   1 - alpha*A;
      a0 =   1 + alpha/A;
      a1 =  -2*cosw0;
      a2 =   1 - alpha/A;
   }
   
   mFF0 = b0/a0;
   mFF1 = b1/a0;
   mFF2 = b2/a0;
   mFB1 = a1/a0;
   mFB2 = a2/a0;
}

float BiquadFilter::Filter(float sample)
{
   float filtered;
   
   FIX_DENORMAL(sample);
   
   filtered = mFF0 * sample + mFF1 * mHistPre1 + mFF2 * mHistPre2 - mFB1 * mHistPost1 - mFB2 * mHistPost2;
   
   FIX_DENORMAL(filtered);
   
   mHistPre2 = mHistPre1;
   mHistPre1 = sample;
   mHistPost2 = mHistPost1;
   mHistPost1 = filtered;
   
   return filtered;
}

void BiquadFilter::Filter(float* buffer, int bufferSize)
{
   for (int i=0; i<bufferSize; ++i)
      buffer[i] = Filter(buffer[i]);
}

void BiquadFilter::CopyCoeffFrom(BiquadFilter& other)
{
   mFF0 = other.mFF0;
   mFF1 = other.mFF1;
   mFF2 = other.mFF2;
   mFB1 = other.mFB1;
   mFB2 = other.mFB2;
}

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
, mQ(sqrt(2) / 2)
, mDbGain(0)
{
   Clear();
   UpdateFilterCoeff();
}

void BiquadFilter::Clear()
{
   mZ1 = 0;
   mZ2 = 0;
}

void BiquadFilter::UpdateFilterCoeff()
{
   if (mF <= 0 || mF != mF)
   {
      mA0 = 1;
      mA1 = 0;
      mA2 = 0;
      mB1 = 0;
      mB2 = 0;
      return;
   }

   double norm;
   double V = pow(10, fabs(mDbGain) / 20.0);
   double K = tan(M_PI * (mF / gSampleRate));
   switch (mType)
   {
      case kFilterType_Lowpass:
         norm = 1 / (1 + K / mQ + K * K);
         mA0 = K * K * norm;
         mA1 = 2 * mA0;
         mA2 = mA0;
         mB1 = 2 * (K * K - 1) * norm;
         mB2 = (1 - K / mQ + K * K) * norm;
         break;

      case kFilterType_Highpass:
         norm = 1 / (1 + K / mQ + K * K);
         mA0 = 1 * norm;
         mA1 = -2 * mA0;
         mA2 = mA0;
         mB1 = 2 * (K * K - 1) * norm;
         mB2 = (1 - K / mQ + K * K) * norm;
         break;

      case kFilterType_Bandpass:
         norm = 1 / (1 + K / mQ + K * K);
         mA0 = K / mQ * norm;
         mA1 = 0;
         mA2 = -mA0;
         mB1 = 2 * (K * K - 1) * norm;
         mB2 = (1 - K / mQ + K * K) * norm;
         break;

      case kFilterType_Notch:
         norm = 1 / (1 + K / mQ + K * K);
         mA0 = (1 + K * K) * norm;
         mA1 = 2 * (K * K - 1) * norm;
         mA2 = mA0;
         mB1 = mA1;
         mB2 = (1 - K / mQ + K * K) * norm;
         break;

      case kFilterType_Peak:
         if (mDbGain >= 0) {    // boost
            norm = 1 / (1 + K / mQ + K * K);
            mA0 = (1 + K / mQ * V + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - K / mQ * V + K * K) * norm;
            mB1 = mA1;
            mB2 = (1 - K / mQ + K * K) * norm;
         }
         else {    // cut
            norm = 1 / (1 + K / mQ * V + K * K);
            mA0 = (1 + K / mQ + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - K / mQ + K * K) * norm;
            mB1 = mA1;
            mB2 = (1 - K / mQ * V + K * K) * norm;
         }
         break;
      case kFilterType_LowShelf:
         if (mDbGain >= 0) {    // boost
            norm = 1 / (1 + K / mQ + K * K);
            mA0 = (1 + sqrt(V) * K / mQ + V * K * K) * norm;
            mA1 = 2 * (V * K * K - 1) * norm;
            mA2 = (1 - sqrt(V) * K / mQ + V * K * K) * norm;
            mB1 = 2 * (K * K - 1) * norm;
            mB2 = (1 - K / mQ + K * K) * norm;
         }
         else {    // cut
            norm = 1 / (1 + sqrt(V) * K / mQ + V * K * K);
            mA0 = (1 + K / mQ + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - K / mQ + K * K) * norm;
            mB1 = 2 * (V * K * K - 1) * norm;
            mB2 = (1 - sqrt(V) * K / mQ + V * K * K) * norm;
         }
         break;
      case kFilterType_HighShelf:
         if (mDbGain >= 0) {    // boost
            norm = 1 / (1 + K / mQ + K * K);
            mA0 = (V + sqrt(V) * K / mQ + K * K) * norm;
            mA1 = 2 * (K * K - V) * norm;
            mA2 = (V - sqrt(V) * K / mQ + K * K) * norm;
            mB1 = 2 * (K * K - 1) * norm;
            mB2 = (1 - K / mQ + K * K) * norm;
         }
         else {    // cut
            norm = 1 / (V + sqrt(V) * K / mQ + K * K);
            mA0 = (1 + K / mQ + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - K / mQ + K * K) * norm;
            mB1 = 2 * (K * K - V) * norm;
            mB2 = (V - sqrt(V) * K / mQ + K * K) * norm;
         }
         break;
      case kFilterType_LowShelfNoQ:
         if (mDbGain >= 0) {    // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            mA0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
            mA1 = 2 * (V * K * K - 1) * norm;
            mA2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
            mB1 = 2 * (K * K - 1) * norm;
            mB2 = (1 - sqrt(2) * K + K * K) * norm;
         }
         else {    // cut
            norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
            mA0 = (1 + sqrt(2) * K + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - sqrt(2) * K + K * K) * norm;
            mB1 = 2 * (V * K * K - 1) * norm;
            mB2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
         }
         break;
      case kFilterType_HighShelfNoQ:
         if (mDbGain >= 0) {    // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            mA0 = (V + sqrt(2 * V) * K + K * K) * norm;
            mA1 = 2 * (K * K - V) * norm;
            mA2 = (V - sqrt(2 * V) * K + K * K) * norm;
            mB1 = 2 * (K * K - 1) * norm;
            mB2 = (1 - sqrt(2) * K + K * K) * norm;
         }
         else {    // cut
            norm = 1 / (V + sqrt(2 * V) * K + K * K);
            mA0 = (1 + sqrt(2) * K + K * K) * norm;
            mA1 = 2 * (K * K - 1) * norm;
            mA2 = (1 - sqrt(2) * K + K * K) * norm;
            mB1 = 2 * (K * K - V) * norm;
            mB2 = (V - sqrt(2 * V) * K + K * K) * norm;
         }
         break;
      case kFilterType_Allpass:
         norm = 1 / (1 + K / mQ + K * K);
         mA0 = (1 - K / mQ + K * K) * norm;
         mA1 = 2 * (K * K - 1) * norm;
         mA2 = 1;
         mB1 = mA1;
         mB2 = mA0;
         break;
      case kFilterType_Off:
         mA0 = 1;
         mA1 = 0;
         mA2 = 0;
         mB1 = 0;
         mB2 = 0;
         break;
   }
}

void BiquadFilter::Filter(float* buffer, int bufferSize)
{
   for (int i=0; i<bufferSize; ++i)
      buffer[i] = Filter(buffer[i]);
}

void BiquadFilter::CopyCoeffFrom(BiquadFilter& other)
{
   mA0 = other.mA0;
   mA1 = other.mA1;
   mA2 = other.mA2;
   mB1 = other.mB1;
   mB2 = other.mB2;
}

float BiquadFilter::GetMagnitudeResponseAt(float f)
{
   auto const piw0 = (f/gSampleRate) * M_PI * 2;
   auto const cosw = std::cos(piw0);
   auto const sinw = std::sin(piw0);

   auto square = [](auto z) { return z * z; };

   auto const numerator = sqrt(square(mA0*square(cosw) - mA0 * square(sinw) + mA1 * cosw + mA2) + square(2 * mA0*cosw*sinw + mA1 * (sinw)));
   auto const denominator = sqrt(square(square(cosw) - square(sinw) + mB1 * cosw + mB2) + square(2 * cosw*sinw + mB1 * (sinw)));

   return numerator / denominator;
}

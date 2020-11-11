//
//  BiquadFilter.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/2/14.
//
//

#ifndef __modularSynth__BiquadFilter__
#define __modularSynth__BiquadFilter__

#include <iostream>

enum FilterType
{
   kFilterType_Off,
   kFilterType_Lowpass,
   kFilterType_Highpass,
   kFilterType_Bandpass,
   kFilterType_Notch,
   kFilterType_Peak,
   kFilterType_LowShelf,
   kFilterType_HighShelf,
   kFilterType_LowShelfNoQ,
   kFilterType_HighShelfNoQ
};

class BiquadFilter
{
public:
   BiquadFilter();
   
   void Clear();
   void SetFilterType(FilterType type) { if (type != mType) { mType = type; Clear(); } }
   void SetFilterParams(double f, double q) { mF = f; mQ = q; UpdateFilterCoeff(); }
   void UpdateFilterCoeff();
   void CopyCoeffFrom(BiquadFilter& other);
   bool UsesGain() { return mType == kFilterType_Peak || mType == kFilterType_HighShelf || mType == kFilterType_LowShelf; }
   bool UsesQ() { return true; }// return mType == kFilterType_Lowpass || mType == kFilterType_Highpass || mType == kFilterType_Bandpass || mType == kFilterType_Notch || mType == kFilterType_Peak; }
   float GetMagnitudeResponseAt(float f);
   
   float Filter(float sample);
   void Filter(float* buffer, int bufferSize);
   
   float mF;
   float mQ;
   float mDbGain;
   FilterType mType;
   
private:
   double mA0;
   double mA1;
   double mA2;
   double mB1;
   double mB2;
   double mZ1;
   double mZ2;
};

inline float BiquadFilter::Filter(float in) {
   double out = in * mA0 + mZ1;
   mZ1 = in * mA1 + mZ2 - mB1 * out;
   mZ2 = in * mA2 - mB2 * out;
   return out;
}

#endif /* defined(__modularSynth__BiquadFilter__) */

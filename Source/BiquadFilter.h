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
   kFilterType_PeakNotch
};

class BiquadFilter
{
public:
   BiquadFilter();
   
   void Clear();
   void SetFilterType(FilterType type) { if (type != mType) { mType = type; Clear(); } }
   void SetFilterParams(float f, float q) { mF = f; mQ = q; UpdateFilterCoeff(); }
   void UpdateFilterCoeff();
   void CopyCoeffFrom(BiquadFilter& other);
   
   float Filter(float sample);
   void Filter(float* buffer, int bufferSize);
   
   float mF;
   float mQ;
   float mDbGain;
   FilterType mType;
   
private:
   float mFF0;
   float mFF1;
   float mFF2;
   float mFB1;
   float mFB2;
   float mHistPre1;
   float mHistPre2;
   float mHistPost1;
   float mHistPost2;
};

#endif /* defined(__modularSynth__BiquadFilter__) */

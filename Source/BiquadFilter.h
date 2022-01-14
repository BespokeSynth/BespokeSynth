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
//  BiquadFilter.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/2/14.
//
//

#pragma once

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
   kFilterType_HighShelfNoQ,
   kFilterType_Allpass
};

class BiquadFilter
{
public:
   BiquadFilter();
   
   void SetSampleRate(double sampleRate) { mSampleRate = sampleRate; }
   void Clear();
   void SetFilterType(FilterType type) { if (type != mType) { mType = type; Clear(); } }
   void SetFilterParams(double f, double q);
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
   double mSampleRate;
};

inline float BiquadFilter::Filter(float in) {
   double out = in * mA0 + mZ1;
   mZ1 = in * mA1 + mZ2 - mB1 * out;
   mZ2 = in * mA2 - mB2 * out;
   return out;
}

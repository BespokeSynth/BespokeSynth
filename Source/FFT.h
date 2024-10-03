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
//  FFT.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/27/13.
//
//

#pragma once

#include "SynthGlobals.h"

// Variables for FFT routine
class FFT
{
public:
   FFT(int nfft);
   ~FFT();
   void Forward(float* input, float* output_re, float* output_im);
   void Inverse(float* input_re, float* input_im, float* output);

private:
   int mNfft{ 0 }; // size of FFT
   int mNumfreqs{ 0 }; // number of frequencies represented (nfft/2 + 1)
   float* mFft_data{ nullptr }; // array for writing/reading to/from FFT function
};

struct FFTData
{
   FFTData(int windowSize, int freqDomainSize)
   : mWindowSize(windowSize)
   , mFreqDomainSize(freqDomainSize)
   {
      mRealValues = new float[freqDomainSize];
      mImaginaryValues = new float[freqDomainSize];
      mTimeDomain = new float[windowSize];
      Clear();
   }

   ~FFTData()
   {
      delete[] mRealValues;
      delete[] mImaginaryValues;
      delete[] mTimeDomain;
   }

   void Clear();

   int mWindowSize{ 0 };
   int mFreqDomainSize{ 0 };
   float* mRealValues{ nullptr };
   float* mImaginaryValues{ nullptr };
   float* mTimeDomain{ nullptr };
};


#ifndef MAYER_H
#define MAYER_H

#define REAL float

void mayer_realfft(int n, REAL* real);
void mayer_realifft(int n, REAL* real);

#endif

//
//  FFT.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/27/13.
//
//

#ifndef __modularSynth__FFT__
#define __modularSynth__FFT__

#include <iostream>

// Variables for FFT routine
class FFT
{
public:
   FFT(int nfft);
   ~FFT();
   void Forward(float* input, float* output_re, float* output_im);
   void Inverse(float* input_re, float* input_im, float* output);
private:
   int mNfft;        // size of FFT
   int mNumfreqs;    // number of frequencies represented (nfft/2 + 1)
   float* mFft_data; // array for writing/reading to/from FFT function
};

struct FFTData
{
   FFTData(int windowSize, int freqDomainSize)
   {
      mRealValues = new float[freqDomainSize];
      mImaginaryValues = new float[freqDomainSize];
      mTimeDomain = new float[windowSize];
   }
   ~FFTData()
   {
      delete[] mRealValues;
      delete[] mImaginaryValues;
      delete[] mTimeDomain;
   }
   float* mRealValues;
   float* mImaginaryValues;
   float* mTimeDomain;
};


#ifndef MAYER_H
#define MAYER_H

#define REAL float

void mayer_realfft(int n, REAL *real);
void mayer_realifft(int n, REAL *real);

#endif



#endif /* defined(__modularSynth__FFT__) */

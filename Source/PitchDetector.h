//
//  PitchDetector.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/14.
//
//

#ifndef __modularSynth__PitchDetector__
#define __modularSynth__PitchDetector__

#include <iostream>

class FFT;

class PitchDetector
{
public:
   PitchDetector();
   ~PitchDetector();
   
   float DetectPitch(float* buffer, int bufferSize);
   
private:
   
   ////////////////////////////////////////
   //ported
   float mTune;
   float mPitch;
   float mConfidence;
   float mLatency;
   ::FFT* mFFT;
   
   unsigned long mfs; // Sample rate
   
   unsigned long mcbsize; // size of circular buffer
   unsigned long mcorrsize; // cbsize/2 + 1
   unsigned long mcbiwr;
   float* mcbi; // circular input buffer
   
   float* mcbwindow; // hann of length N/2, zeros for the rest
   float* macwinv; // inverse of autocorrelation of window
   int mnoverlap;
   
   float* mffttime;
   float* mfftfreqre;
   float* mfftfreqim;
   
   // VARIABLES FOR LOW-RATE SECTION
   float maref; // A tuning reference (Hz)
   float mconf; // Confidence of pitch period estimate (between 0 and 1)
   float mvthresh; // Voiced speech threshold
   
   float mpmax; // Maximum allowable pitch period (seconds)
   float mpmin; // Minimum allowable pitch period (seconds)
   unsigned long mnmax; // Maximum period index for pitch prd est
   unsigned long mnmin; // Minimum period index for pitch prd est
   
   float mlrshift; // Shift prescribed by low-rate section
   int mptarget; // Pitch target, between 0 and 11
   float msptarget; // Smoothed pitch target
};

#endif /* defined(__modularSynth__PitchDetector__) */

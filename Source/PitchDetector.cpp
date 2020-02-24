//
//  PitchDetector.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/14.
//
//

#include "PitchDetector.h"
#include "FFT.h"
#include "SynthGlobals.h"

#define L2SC (float)3.32192809488736218171

PitchDetector::PitchDetector()
: mTune(440)
, mPitch(0)
, mConfidence(0)
{
   mfs = gSampleRate;
   
   mcbsize = 2048;
   mcorrsize = mcbsize / 2 + 1;
   
   mpmax = 1/(float)70;  // max and min periods (ms)
   mpmin = 1/(float)700; // eventually may want to bring these out as sliders
   
   mnmax = (unsigned long)(gSampleRate * mpmax);
   if (mnmax > mcorrsize) {
      mnmax = mcorrsize;
   }
   mnmin = (unsigned long)(gSampleRate * mpmin);
   
   mcbi = (float*)calloc(mcbsize, sizeof(float));
   
   mcbiwr = 0;
   
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mcbwindow = (float*)calloc(mcbsize, sizeof(float));
   for (int ti=0; ti<(mcbsize / 2); ti++) {
      mcbwindow[ti+mcbsize/4] = -0.5*cos(4*PI*ti/(mcbsize - 1)) + 0.5;
   }
   
   mnoverlap = 4;
   
   mFFT = new ::FFT((int)mcbsize);
   
   mffttime = (float*)calloc(mcbsize, sizeof(float));
   mfftfreqre = (float*)calloc(mcorrsize, sizeof(float));
   mfftfreqim = (float*)calloc(mcorrsize, sizeof(float));
   
   
   // ---- Calculate autocorrelation of window ----
   macwinv = (float*)calloc(mcbsize, sizeof(float));
   for (int ti=0; ti<mcbsize; ti++) {
      mffttime[ti] = mcbwindow[ti];
   }
   mFFT->Forward(mcbwindow, mfftfreqre, mfftfreqim);
   for (int ti=0; ti<mcorrsize; ti++) {
      mfftfreqre[ti] = (mfftfreqre[ti])*(mfftfreqre[ti]) + (mfftfreqim[ti])*(mfftfreqim[ti]);
      mfftfreqim[ti] = 0;
   }
   mFFT->Inverse(mfftfreqre, mfftfreqim, mffttime);
   for (long ti=1; ti<mcbsize; ti++) {
      macwinv[ti] = mffttime[ti]/mffttime[0];
      if (macwinv[ti] > 0.000001) {
         macwinv[ti] = (float)1/macwinv[ti];
      }
      else {
         macwinv[ti] = 0;
      }
   }
   macwinv[0] = 1;
   // ---- END Calculate autocorrelation of window ----
   
   
   mlrshift = 0;
   mptarget = 0;
   msptarget = 0;
   
   mvthresh = 0.7;  //  The voiced confidence (unbiased peak) threshold level
}

PitchDetector::~PitchDetector()
{
   delete mFFT;
   free(mcbi);
   free(mcbwindow);
   free(macwinv);
   free(mffttime);
   free(mfftfreqre);
   free(mfftfreqim);
}

float PitchDetector::DetectPitch(float* buffer, int bufferSize)
{
   long int N;
   long int Nf;
   long int fs;
   
   long int ti;
   long int ti2;
   long int ti3;
   long int ti4;
   float tf;
   float tf2;
   
   float pperiod;
   
   float* pfInput = buffer;
   
   maref = (float)mTune;
   
   N = mcbsize;
   Nf = mcorrsize;
   fs = mfs;
   
   float inpitch;
   float conf = mconf;
   
   
   /*******************
    *  MAIN DSP LOOP  *
    *******************/
   for (int lSampleIndex = 0; lSampleIndex < bufferSize; lSampleIndex++)
   {
      // load data into circular buffer
      tf = (float) *(pfInput++);
      ti4 = mcbiwr;
      mcbi[ti4] = tf;
      
      // Input write pointer logic
      mcbiwr++;
      if (mcbiwr >= N)
      {
         mcbiwr = 0;
      }
      
      
      // ********************
      // * Low-rate section *
      // ********************
      
      // Every N/noverlap samples, run pitch estimation / manipulation code
      if ((mcbiwr)%(N/mnoverlap) == 0)
      {
         // ---- Obtain autocovariance ----
         
         // Window and fill FFT buffer
         ti2 = mcbiwr;
         for (ti=0; ti<N; ti++)
         {
            mffttime[ti] = (float)(mcbi[(ti2-ti+N)%N]*mcbwindow[ti]);
         }
         
         // Calculate FFT
         mFFT->Forward(mffttime, mfftfreqre, mfftfreqim);
         
         // Remove DC
         mfftfreqre[0] = 0;
         mfftfreqim[0] = 0;
         
         // Take magnitude squared
         for (ti=1; ti<Nf; ti++)
         {
            mfftfreqre[ti] = (mfftfreqre[ti])*(mfftfreqre[ti]) + (mfftfreqim[ti])*(mfftfreqim[ti]);
            mfftfreqim[ti] = 0;
         }
         
         // Calculate IFFT
         mFFT->Inverse(mfftfreqre, mfftfreqim, mffttime);
         
         // Normalize
         tf = (float)1/mffttime[0];
         for (ti=1; ti<N; ti++)
         {
            mffttime[ti] = mffttime[ti] * tf;
         }
         mffttime[0] = 1;
         
         //  ---- END Obtain autocovariance ----
         
         
         //  ---- Calculate pitch and confidence ----
         
         // Calculate pitch period
         //   Pitch period is determined by the location of the max (biased)
         //     peak within a given range
         //   Confidence is determined by the corresponding unbiased height
         tf2 = 0;
         pperiod = mpmin;
         for (ti=mnmin; ti<mnmax; ti++)
         {
            ti2 = ti-1;
            ti3 = ti+1;
            if (ti2<0)
            {
               ti2 = 0;
            }
            if (ti3>Nf)
            {
               ti3 = Nf;
            }
            tf = mffttime[ti];
            
            if (tf>mffttime[ti2] && tf>=mffttime[ti3] && tf>tf2)
            {
               tf2 = tf;
               ti4 = ti;
            }
         }
         if (tf2>0)
         {
            conf = tf2*macwinv[ti4];
            if (ti4>0 && ti4<Nf)
            {
               // Find the center of mass in the vicinity of the detected peak
               tf = mffttime[ti4-1]*(ti4-1);
               tf = tf + mffttime[ti4]*(ti4);
               tf = tf + mffttime[ti4+1]*(ti4+1);
               tf = tf/(mffttime[ti4-1] + mffttime[ti4] + mffttime[ti4+1]);
               pperiod = tf/fs;
            }
            else
            {
               pperiod = (float)ti4/fs;
            }
         }
         
         // Convert to semitones
         tf = (float) -12*log10((float)maref*pperiod)*L2SC;
         if (conf>=mvthresh)
         {
            inpitch = tf;
         }
         mconf = conf;
         
         mPitch = inpitch + 69;
         mConfidence = MIN(conf,1);
         
         //  ---- END Calculate pitch and confidence ----
      }
   }
   
   // Tell the host the algorithm latency
   mLatency = (N-1);
   
   return mPitch;
}

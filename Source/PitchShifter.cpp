//
//  PitchShifter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#include "PitchShifter.h"
#include "SynthGlobals.h"
#include "Profiler.h"

PitchShifter::PitchShifter(int fftBins)
: mFFTBins(fftBins)
, mFFT(mFFTBins)
, mRollingInputBuffer(mFFTBins)
, mRollingOutputBuffer(mFFTBins)
, mFFTData(mFFTBins, mFFTBins/2 + 1)
, mRatio(1)
, gRover(false)
, gInit(false)
, mLatency(0)
, mOversampling(4)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[mFFTBins];
   for (int i=0; i<mFFTBins; ++i)
      mWindower[i] = -.5*cos(FTWO_PI*i/mFFTBins)+.5;
   mLastPhase = new float[mFFTBins/2+1];
   mSumPhase = new float[mFFTBins/2+1];
   mAnalysisMag = new float[mFFTBins];
   mAnalysisFreq = new float[mFFTBins];
   mSynthesisMag = new float[mFFTBins];
   mSynthesisFreq = new float[mFFTBins];
}

PitchShifter::~PitchShifter()
{
   delete[] mLastPhase;
   delete[] mSumPhase;
   delete[] mWindower;
   delete[] mAnalysisMag;
   delete[] mAnalysisFreq;
   delete[] mSynthesisMag;
   delete[] mSynthesisFreq;
}

#define MY_PITCHSHIFTER 0

#if MY_PITCHSHIFTER

void PitchShifter::Process(float* buffer, int bufferSize)
{
   PROFILER(PitchShifter);
   
   const int osamp = mOversampling;
   const int stepSize = mFFTBins/osamp;
   const double expct = 2.*M_PI*(double)stepSize/(double)mFFTBins;
   const double freqPerBin = gSampleRate/(double)mFFTBins;
   
   mLatency = mFFTBins-stepSize;
   
   mRollingInputBuffer.WriteChunk(buffer, bufferSize, 0);
   
   //copy rolling input buffer into working buffer and window it
   mRollingInputBuffer.ReadChunk(mFFTData.mTimeDomain, mFFTBins, latency);
   Mult(mFFTData.mTimeDomain, mWindower, mFFTBins);
   
   mFFT.Forward(mFFTData.mTimeDomain,
                mFFTData.mRealValues,
                mFFTData.mImaginaryValues);

   const int fftFrameSize2 = mFFTBins/2;

   // this is the analysis step
   for (int k = 0; k <= fftFrameSize2; k++)
   {
      // de-interlace FFT buffer
      float real = mFFTData.mRealValues[k];
      float imag = mFFTData.mImaginaryValues[k];
      
      // compute magnitude and phase
      double mag = 2.*sqrt(real*real + imag*imag);
      double phase = atan2(imag,real);
      
      // compute phase difference 
      double diff = phase - mLastPhase[k];
      mLastPhase[k] = phase;
      
      // subtract expected phase difference 
      diff -= (double)k*expct;
      
      // map delta phase into +/- Pi interval
      long qpd = diff/M_PI;
      if (qpd >= 0) qpd += qpd&1;
      else qpd -= qpd&1;
      diff -= M_PI*(double)qpd;
      
      // get deviation from bin frequency from the +/- Pi interval 
      double deviation = osamp*diff/(2.*M_PI);
      
      // compute the k-th partials' true frequency 
      double freq = (double)k*freqPerBin + deviation*freqPerBin;
      
      // store magnitude and true frequency in analysis arrays 
      mAnalysisMag[k] = mag;
      mAnalysisFreq[k] = freq;
   }
   
   // this does the actual pitch shifting 
   memset(mSynthesisMag, 0, mFFTBins*sizeof(float));
   memset(mSynthesisFreq, 0, mFFTBins*sizeof(float));
   for (int k = 0; k <= fftFrameSize2; k++)
   {
      int index = k*mRatio;
      if (index <= fftFrameSize2)
      {
         mSynthesisMag[index] += mAnalysisMag[k];
         mSynthesisFreq[index] = mAnalysisFreq[k] * mRatio;
      } 
   }
   
   // this is the synthesis step 
   for (int k = 0; k <= fftFrameSize2; k++)
   {
      // get magnitude and true frequency from synthesis arrays 
      double mag = mSynthesisMag[k];
      double tmp = mSynthesisFreq[k];
      
      // subtract bin mid frequency 
      tmp -= (double)k*freqPerBin;
      
      // get bin deviation from freq deviation 
      tmp /= freqPerBin;
      
      // take osamp into account 
      tmp = 2.*M_PI*tmp/osamp;
      
      // add the overlap phase advance back in 
      tmp += (double)k*expct;
      
      // accumulate delta phase to get bin phase 
      mSumPhase[k] += tmp;
      double phase = mSumPhase[k];
      
      // get real and imag part and re-interleave 
      mFFTData.mRealValues[k+1] = mag*cos(phase);
      mFFTData.mImaginaryValues[k+1] = mag*sin(phase);
   }
   
   mFFT.Inverse(mFFTData.mRealValues,
                mFFTData.mImaginaryValues,
                mFFTData.mTimeDomain);
   
   for (int i=0; i<bufferSize; ++i)
      mRollingOutputBuffer.Write(0);
   
   //copy rolling input buffer into working buffer and window it
   for (int i=0; i<mFFTBins; ++i)
      mRollingOutputBuffer.Accum(mFFTBins-i, mFFTData.mTimeDomain[i] * mWindower[i] * .0001f);
   
   for (int i=0; i<bufferSize; ++i)
      buffer[i] = mRollingOutputBuffer.GetSample(latency-i);
}

#else

void smbFft(float *fftBuffer, long fftFrameSize, long sign)
/*
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
 */
{
   float wr, wi, arg, *p1, *p2, temp;
   float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
   long i, bitm, j, le, le2, k;
   
   for (i = 2; i < 2*fftFrameSize-2; i += 2) {
      for (bitm = 2, j = 0; bitm < 2*fftFrameSize; bitm <<= 1) {
         if (i & bitm) j++;
         j <<= 1;
      }
      if (i < j) {
         p1 = fftBuffer+i; p2 = fftBuffer+j;
         temp = *p1; *(p1++) = *p2;
         *(p2++) = temp; temp = *p1;
         *p1 = *p2; *p2 = temp;
      }
   }
   for (k = 0, le = 2; k < (long)(log(fftFrameSize)/log(2.)+.5); k++) {
      le <<= 1;
      le2 = le>>1;
      ur = 1.0;
      ui = 0.0;
      arg = M_PI / (le2>>1);
      wr = cos(arg);
      wi = sign*sin(arg);
      for (j = 0; j < le2; j += 2) {
         p1r = fftBuffer+j; p1i = p1r+1;
         p2r = p1r+le2; p2i = p2r+1;
         for (i = j; i < 2*fftFrameSize; i += le) {
            tr = *p2r * ur - *p2i * ui;
            ti = *p2r * ui + *p2i * ur;
            *p2r = *p1r - tr; *p2i = *p1i - ti;
            *p1r += tr; *p1i += ti;
            p1r += le; p1i += le;
            p2r += le; p2i += le;
         }
         tr = ur*wr - ui*wi;
         ui = ur*wi + ui*wr;
         ur = tr;
      }
   }
}

/****************************************************************************
 *
 * NAME: smbPitchShift.cpp
 * VERSION: 1.2
 * HOME URL: http://blogs.zynaptiq.com/bernsee
 * KNOWN BUGS: none
 *
 * SYNOPSIS: Routine for doing pitch shifting while maintaining
 * duration using the Short Time Fourier Transform.
 *
 * DESCRIPTION: The routine takes a pitchShift factor value which is between 0.5
 * (one octave down) and 2. (one octave up). A value of exactly 1 does not change
 * the pitch. numSampsToProcess tells the routine how many samples in indata[0...
 * numSampsToProcess-1] should be pitch shifted and moved to outdata[0 ...
 * numSampsToProcess-1]. The two buffers can be identical (ie. it can process the
 * data in-place). fftFrameSize defines the FFT frame size used for the
 * processing. Typical values are 1024, 2048 and 4096. It may be any value <=
 * MAX_FRAME_LENGTH but it MUST be a power of 2. osamp is the STFT
 * oversampling factor which also determines the overlap between adjacent STFT
 * frames. It should at least be 4 for moderate scaling ratios. A value of 32 is
 * recommended for best quality. sampleRate takes the sample rate for the signal
 * in unit Hz, ie. 44100 for 44.1 kHz audio. The data passed to the routine in
 * indata[] should be in the range [-1.0, 1.0), which is also the output range
 * for the data, make sure you scale the data accordingly (for 16bit signed integers
 * you would have to divide (and multiply) by 32768).
 *
 * COPYRIGHT 1999-2015 Stephan M. Bernsee <s.bernsee [AT] zynaptiq [DOT] com>
 *
 * 						The Wide Open License (WOL)
 *
 * Permission to use, copy, modify, distribute and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice and this license appear in all source copies.
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
 * ANY KIND. See http://www.dspguru.com/wol.htm for more information.
 *
 *****************************************************************************/

void PitchShifter::Process(float* buffer, int bufferSize)
{
   PROFILER(PitchShifter);
   
   const int fftFrameSize = mFFTBins;
   const int sampleRate = gSampleRate;
   const int osamp = mOversampling;
   const int numSampsToProcess = bufferSize;
   float* indata = buffer;
   float* outdata = buffer;
   const float pitchShift = mRatio;

   double magn, phase, tmp, window, real, imag;
   double freqPerBin, expct;
   long i,k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;
   
   /* set up some handy variables */
   fftFrameSize2 = fftFrameSize/2;
   stepSize = fftFrameSize/osamp;
   freqPerBin = sampleRate/(double)fftFrameSize;
   expct = 2.*M_PI*(double)stepSize/(double)fftFrameSize;
   inFifoLatency = fftFrameSize-stepSize;
   if (gRover == false)
      gRover = inFifoLatency;
   
   mLatency = inFifoLatency;
   
   /* initialize our static arrays */
   if (gInit == false)
   {
      memset(gInFIFO, 0, MAX_FRAME_LENGTH*sizeof(float));
      memset(gOutFIFO, 0, MAX_FRAME_LENGTH*sizeof(float));
      memset(gFFTworksp, 0, 2*MAX_FRAME_LENGTH*sizeof(float));
      memset(gLastPhase, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(float));
      memset(gSumPhase, 0, (MAX_FRAME_LENGTH/2+1)*sizeof(float));
      memset(gOutputAccum, 0, 2*MAX_FRAME_LENGTH*sizeof(float));
      memset(gAnaFreq, 0, MAX_FRAME_LENGTH*sizeof(float));
      memset(gAnaMagn, 0, MAX_FRAME_LENGTH*sizeof(float));
      gInit = true;
   }
   
   /* main processing loop */
   for (i = 0; i < numSampsToProcess; i++)
   {
      /* As long as we have not yet collected enough data just read in */
      gInFIFO[gRover] = indata[i];
      outdata[i] = gOutFIFO[gRover-inFifoLatency];
      gRover++;
      
      /* now we have enough data for processing */
      if (gRover >= fftFrameSize)
      {
         gRover = inFifoLatency;
         
         /* do windowing and re,im interleave */
         for (k = 0; k < fftFrameSize;k++)
         {
            window = -.5*cos(2.*M_PI*(double)k/(double)fftFrameSize)+.5;
            gFFTworksp[2*k] = gInFIFO[k] * window;
            gFFTworksp[2*k+1] = 0.;
         }
         
         
         /* ***************** ANALYSIS ******************* */
         /* do transform */
         smbFft(gFFTworksp, fftFrameSize, -1);
         
         /* this is the analysis step */
         for (k = 0; k <= fftFrameSize2; k++)
         {
            /* de-interlace FFT buffer */
            real = gFFTworksp[2*k];
            imag = gFFTworksp[2*k+1];
            
            /* compute magnitude and phase */
            magn = 2.*sqrt(real*real + imag*imag);
            phase = atan2(imag,real);
            
            /* compute phase difference */
            tmp = phase - gLastPhase[k];
            gLastPhase[k] = phase;
            
            /* subtract expected phase difference */
            tmp -= (double)k*expct;
            
            /* map delta phase into +/- Pi interval */
            qpd = tmp/M_PI;
            if (qpd >= 0) qpd += qpd&1;
            else qpd -= qpd&1;
            tmp -= M_PI*(double)qpd;
            
            /* get deviation from bin frequency from the +/- Pi interval */
            tmp = osamp*tmp/(2.*M_PI);
            
            /* compute the k-th partials' true frequency */
            tmp = (double)k*freqPerBin + tmp*freqPerBin;
            
            /* store magnitude and true frequency in analysis arrays */
            gAnaMagn[k] = magn;
            gAnaFreq[k] = tmp;
            
         }
         
         /* ***************** PROCESSING ******************* */
         /* this does the actual pitch shifting */
         memset(gSynMagn, 0, fftFrameSize*sizeof(float));
         memset(gSynFreq, 0, fftFrameSize*sizeof(float));
         for (k = 0; k <= fftFrameSize2; k++)
         {
            index = k*pitchShift;
            if (index <= fftFrameSize2)
            {
               gSynMagn[index] += gAnaMagn[k];
               gSynFreq[index] = gAnaFreq[k] * pitchShift;
            }
         }
         
         /* ***************** SYNTHESIS ******************* */
         /* this is the synthesis step */
         for (k = 0; k <= fftFrameSize2; k++)
         {
            /* get magnitude and true frequency from synthesis arrays */
            magn = gSynMagn[k];
            tmp = gSynFreq[k];
            
            /* subtract bin mid frequency */
            tmp -= (double)k*freqPerBin;
            
            /* get bin deviation from freq deviation */
            tmp /= freqPerBin;
            
            /* take osamp into account */
            tmp = 2.*M_PI*tmp/osamp;
            
            /* add the overlap phase advance back in */
            tmp += (double)k*expct;
            
            /* accumulate delta phase to get bin phase */
            gSumPhase[k] += tmp;
            phase = gSumPhase[k];
            
            /* get real and imag part and re-interleave */
            gFFTworksp[2*k] = magn*cos(phase);
            gFFTworksp[2*k+1] = magn*sin(phase);
         }
         
         /* zero negative frequencies */
         for (k = fftFrameSize+2; k < 2*fftFrameSize; k++)
            gFFTworksp[k] = 0.;
         
         /* do inverse transform */
         smbFft(gFFTworksp, fftFrameSize, 1);
         
         /* do windowing and add to output accumulator */
         for(k=0; k < fftFrameSize; k++)
         {
            window = -.5*cos(2.*M_PI*(double)k/(double)fftFrameSize)+.5;
            gOutputAccum[k] += 2.*window*gFFTworksp[2*k]/(fftFrameSize2*osamp);
         }
         for (k = 0; k < stepSize; k++) gOutFIFO[k] = gOutputAccum[k];
         
         /* shift accumulator */
         memmove(gOutputAccum, gOutputAccum+stepSize, fftFrameSize*sizeof(float));
         
         /* move input FIFO */
         for (k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k+stepSize];
      }
   }
}

#endif


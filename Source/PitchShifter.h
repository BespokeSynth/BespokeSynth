//
//  PitchShifter.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#ifndef __Bespoke__PitchShifter__
#define __Bespoke__PitchShifter__

#include <iostream>
#include "FFT.h"
#include "RollingBuffer.h"

#define MAX_FRAME_LENGTH 8192

class PitchShifter
{
public:
   PitchShifter(int fftBins);
   virtual ~PitchShifter();
   
   void Process(float* buffer, int bufferSize);
   void SetRatio(float ratio) { mRatio = ratio; }
   void SetOversampling(int oversampling) { mOversampling = oversampling; }
   int GetLatency() const { return mLatency; }
   
private:
   int mFFTBins;
   
   FFTData mFFTData;
   
   float* mLastPhase;
   float* mSumPhase;
   float* mWindower;
   float mRover;
   float* mAnalysisMag;
   float* mAnalysisFreq;
   float* mSynthesisMag;
   float* mSynthesisFreq;

   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;
   
   float mRatio;
   int mOversampling;
   float mLatency;
   
   float gInFIFO[MAX_FRAME_LENGTH];
   float gOutFIFO[MAX_FRAME_LENGTH];
   float gFFTworksp[2*MAX_FRAME_LENGTH];
   float gLastPhase[MAX_FRAME_LENGTH/2+1];
   float gSumPhase[MAX_FRAME_LENGTH/2+1];
   float gOutputAccum[2*MAX_FRAME_LENGTH];
   float gAnaFreq[MAX_FRAME_LENGTH];
   float gAnaMagn[MAX_FRAME_LENGTH];
   float gSynFreq[MAX_FRAME_LENGTH];
   float gSynMagn[MAX_FRAME_LENGTH];
   long gRover;
   long gInit;
};

#endif /* defined(__Bespoke__PitchShifter__) */

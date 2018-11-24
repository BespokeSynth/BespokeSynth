//
//  MultiBandTracker.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#include "MultiBandTracker.h"
#include "SynthGlobals.h"
#include "Profiler.h"

MultiBandTracker::MultiBandTracker()
{
   mMinFreq = 150;
   mMaxFreq = 15000;
   SetNumBands(8);
   
   mWorkBuffer = new float[gBufferSize];
}

MultiBandTracker::~MultiBandTracker()
{
   delete[] mWorkBuffer;
}

void MultiBandTracker::SetRange(float minFreq, float maxFreq)
{
   for (int i=0; i<mNumBands; ++i)
   {
      float a = float(i)/mNumBands;
      float f = mMinFreq * powf(mMaxFreq/mMinFreq, a);
      mBands[i].SetCrossoverFreq(f);
   }
}

void MultiBandTracker::SetNumBands(int numBands)
{
   mMutex.lock();
   mNumBands = numBands;
   mBands.resize(numBands);
   mPeaks.resize(numBands);
   SetRange(mMinFreq, mMaxFreq);
   mMutex.unlock();
}

void MultiBandTracker::Process(float* buffer, int bufferSize)
{
   PROFILER(MultiBandTracker);

   mMutex.lock();
   
   for (int i=0; i<bufferSize; ++i)
   {
      float lower;
      float highLeftover = buffer[i];
      for (int j=0; j<mNumBands; ++j)
      {
         mBands[j].ProcessSample(highLeftover, lower, highLeftover);
         mPeaks[j].Process(&lower, 1);
      }
   }
   
   mMutex.unlock();
}

float MultiBandTracker::GetBand(int idx)
{
   return mPeaks[idx].GetPeak();
}

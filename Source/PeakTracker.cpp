//
//  PeakTracker.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/4/14.
//
//

#include "PeakTracker.h"
#include "SynthGlobals.h"
#include "Profiler.h"

void PeakTracker::Process(float* buffer, int bufferSize)
{
   PROFILER(PeakTracker);

   for (int j=0; j<bufferSize; ++j)
   {
      float scalar = powf( 0.5f, 1.0f/(mDecayTime * gSampleRate));
      float input = fabsf(buffer[j]);
      
      if ( input >= mPeak )
      {
         /* When we hit a peak, ride the peak to the top. */
         mPeak = input;
         if (mLimit != -1)
            mPeak = ofClamp(mPeak, 0, mLimit);
      }
      else
      {
         /* Exponential decay of output when signal is low. */
         mPeak = mPeak * scalar;
         if(mPeak < FLT_EPSILON)
            mPeak = 0.0;
      }
   }
}

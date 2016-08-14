//
//  PeakTracker.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/4/14.
//
//

#ifndef __modularSynth__PeakTracker__
#define __modularSynth__PeakTracker__

#include <iostream>

class PeakTracker
{
public:
   PeakTracker() : mPeak(0), mDecayTime(.01f), mLimit(-1) {}
   
   void Process(float* buffer, int bufferSize);
   float GetPeak() const { return mPeak; }
   void SetDecayTime(float time) { mDecayTime = time; }
   void SetLimit(float limit) { mLimit = limit; }
   void Reset() { mPeak = 0; }
   
private:
   float mPeak;
   float mDecayTime;
   float mLimit;
};

#endif /* defined(__modularSynth__PeakTracker__) */

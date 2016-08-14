//
//  MultiBandTracker.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#ifndef __modularSynth__MultiBandTracker__
#define __modularSynth__MultiBandTracker__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "LinkwitzRileyFilter.h"
#include "PeakTracker.h"

class MultiBandTracker
{
public:
   MultiBandTracker();
   ~MultiBandTracker();
   
   void SetRange(float minFreq, float maxFreq);
   void SetNumBands(int numBands);
   void Process(float* buffer, int bufferSize);
   float GetBand(int idx);
   int NumBands() { return mNumBands; }
private:
   vector<CLinkwitzRiley_4thOrder> mBands;
   vector<PeakTracker> mPeaks;
   int mNumBands;
   float mMinFreq;
   float mMaxFreq;
   float* mWorkBuffer;
   ofMutex mMutex;
};

#endif /* defined(__modularSynth__MultiBandTracker__) */

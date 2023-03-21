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
   SetNumBands(8);

   mWorkBuffer = new float[gBufferSize];
}

MultiBandTracker::~MultiBandTracker()
{
   delete[] mWorkBuffer;
}

void MultiBandTracker::SetRange(float minFreq, float maxFreq)
{
   for (int i = 0; i < mNumBands; ++i)
   {
      float a = float(i) / mNumBands;
      float f = mMinFreq * powf(mMaxFreq / mMinFreq, a);
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

   for (int i = 0; i < bufferSize; ++i)
   {
      float lower;
      float highLeftover = buffer[i];
      for (int j = 0; j < mNumBands; ++j)
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

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

   for (int j = 0; j < bufferSize; ++j)
   {
      float scalar = powf(0.5f, 1.0f / (mDecayTime * gSampleRate));
      float input = fabsf(buffer[j]);

      if (input >= mPeak)
      {
         /* When we hit a peak, ride the peak to the top. */
         mPeak = input;
         if (mLimit > std::numeric_limits<float>::epsilon() && mPeak >= mLimit)
         {
            mPeak = mLimit;
            mHitLimitTime = gTime;
         }
      }
      else
      {
         /* Exponential decay of output when signal is low. */
         mPeak = mPeak * scalar;
         if (mPeak < std::numeric_limits<float>::epsilon())
            mPeak = 0.0;
      }
   }
}

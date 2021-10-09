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
   PeakTracker() : mPeak(0), mDecayTime(.01f), mLimit(-1), mHitLimitTime(-9999) {}
   
   void Process(float* buffer, int bufferSize);
   float GetPeak() const { return mPeak; }
   void SetDecayTime(float time) { mDecayTime = time; }
   void SetLimit(float limit) { mLimit = limit; }
   void Reset() { mPeak = 0; }
   double GetLastHitLimitTime() const { return mHitLimitTime; }
   
private:
   float mPeak;
   float mDecayTime;
   float mLimit;
   double mHitLimitTime;
};

#endif /* defined(__modularSynth__PeakTracker__) */

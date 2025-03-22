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
//  MultiBandTracker.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#pragma once

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
   std::vector<CLinkwitzRiley_4thOrder> mBands;
   std::vector<PeakTracker> mPeaks;
   int mNumBands{ 8 };
   float mMinFreq{ 150 };
   float mMaxFreq{ 15000 };
   float* mWorkBuffer{ nullptr };
   ofMutex mMutex;
};

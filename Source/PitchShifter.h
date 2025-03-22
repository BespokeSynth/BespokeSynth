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
//  PitchShifter.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#pragma once

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

   float* mLastPhase{ nullptr };
   float* mSumPhase{ nullptr };
   float* mWindower{ nullptr };
   float* mAnalysisMag{ nullptr };
   float* mAnalysisFreq{ nullptr };
   float* mSynthesisMag{ nullptr };
   float* mSynthesisFreq{ nullptr };

   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;

   float mRatio{ 1 };
   int mOversampling{ 4 };
   float mLatency{ 0 };

   float gInFIFO[MAX_FRAME_LENGTH]{};
   float gOutFIFO[MAX_FRAME_LENGTH]{};
   float gFFTworksp[2 * MAX_FRAME_LENGTH]{};
   float gLastPhase[MAX_FRAME_LENGTH / 2 + 1]{};
   float gSumPhase[MAX_FRAME_LENGTH / 2 + 1]{};
   float gOutputAccum[2 * MAX_FRAME_LENGTH]{};
   float gAnaFreq[MAX_FRAME_LENGTH]{};
   float gAnaMagn[MAX_FRAME_LENGTH]{};
   float gSynFreq[MAX_FRAME_LENGTH]{};
   float gSynMagn[MAX_FRAME_LENGTH]{};
   long gRover{ false };
   long gInit{ false };
};

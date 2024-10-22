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
//  PitchDetector.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/14.
//
//

#pragma once

class FFT;

class PitchDetector
{
public:
   PitchDetector();
   ~PitchDetector();

   float DetectPitch(float* buffer, int bufferSize);

private:
   ////////////////////////////////////////
   //ported
   float mTune{ 440 };
   float mPitch{ 0 };
   float mConfidence{ 0 };
   float mLatency{ 0 };
   ::FFT* mFFT;

   unsigned long mfs; // Sample rate

   unsigned long mcbsize; // size of circular buffer
   unsigned long mcorrsize; // cbsize/2 + 1
   unsigned long mcbiwr;
   float* mcbi; // circular input buffer

   float* mcbwindow; // hann of length N/2, zeros for the rest
   float* macwinv; // inverse of autocorrelation of window
   int mnoverlap;

   float* mffttime;
   float* mfftfreqre;
   float* mfftfreqim;

   // VARIABLES FOR LOW-RATE SECTION
   float maref{ 440 }; // A tuning reference (Hz)
   float mconf{ 0 }; // Confidence of pitch period estimate (between 0 and 1)
   float mvthresh; // Voiced speech threshold

   float mpmax; // Maximum allowable pitch period (seconds)
   float mpmin; // Minimum allowable pitch period (seconds)
   unsigned long mnmax; // Maximum period index for pitch prd est
   unsigned long mnmin; // Minimum period index for pitch prd est

   float mlrshift; // Shift prescribed by low-rate section
   int mptarget; // Pitch target, between 0 and 11
   float msptarget; // Smoothed pitch target
};

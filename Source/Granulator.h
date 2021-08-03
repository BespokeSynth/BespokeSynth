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
//  Granulator.h
//  modularSynth
//
//  Created by Ryan Challinor on 9/12/13.
//
//

#ifndef __modularSynth__Granulator__
#define __modularSynth__Granulator__

#include <iostream>
#include "Ramp.h"
#include "BiquadFilter.h"
#include "ChannelBuffer.h"

#define MAX_GRAINS 32

class Granulator;

class Grain
{
public:
   Grain() : mPos(0), mSpeedMult(1), mStartTime(0), mEndTime(0), mVol(0), mStereoPosition(0) {}
   void Spawn(Granulator* owner, double time, double pos, float speedMult, float lengthInMs, float vol, float width);
   void Process(double time, ChannelBuffer* buffer, int bufferLength, float* output);
   void DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength);
   void Clear() { mVol = 0; }
private:
   double GetWindow(double time);
   double mPos;
   float mSpeedMult;
   double mStartTime;
   double mEndTime;
   float mVol;
   float mStereoPosition;
   float mDrawPos;
   Granulator* mOwner;
};

class Granulator
{
public:
   Granulator();
   void ProcessFrame(double time, ChannelBuffer* buffer, int bufferLength, double offset, float* output);
   void Draw(float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength);
   void Reset();
   void ClearGrains();
   void SetLiveMode(bool live) { mLiveMode = live; }
   
   float mSpeed;
   float mGrainLengthMs;
   float mGrainOverlap;
   float mPosRandomizeMs;
   float mSpeedRandomize;
   float mSpacingRandomize;
   bool mOctaves;
   float mWidth;
   
private:
   void SpawnGrain(double time, double offset, float width);
   
   double mNextGrainSpawnMs;
   int mNextGrainIdx;
   Grain mGrains[MAX_GRAINS];
   bool mLiveMode;
   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels];
};

#endif /* defined(__modularSynth__Granulator__) */

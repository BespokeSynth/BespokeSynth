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

#pragma once

#include "BiquadFilter.h"
#include "ChannelBuffer.h"

#define MAX_GRAINS 32

class Granulator;

class Grain
{
public:
   void Spawn(Granulator* owner, double time, double pos, float speedMult, float lengthInMs, float vol, float width);
   void Process(double time, ChannelBuffer* buffer, int bufferLength, float* output);
   void DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength);
   void Clear() { mVol = 0; }

private:
   double GetWindow(double time);
   double mPos{ 0 };
   float mSpeedMult{ 1 };
   double mStartTime{ 0 };
   double mEndTime{ 1 };
   double mStartToEnd{ 1 }, mStartToEndInv{ 1 };
   float mVol{ 0 };
   float mStereoPosition{ 0 };
   float mDrawPos{ .5 };
   Granulator* mOwner{ nullptr };
};

class Granulator
{
public:
   Granulator();
   void ProcessFrame(double time, ChannelBuffer* buffer, int bufferLength, double offset, float speed, float* output);
   void Draw(float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength);
   void Reset();
   void ClearGrains();
   void SetLiveMode(bool live) { mLiveMode = live; }

   float mSpeed{ 1 };
   float mGrainLengthMs{ 60 };
   float mGrainOverlap{ 10 };
   float mPosRandomizeMs{ 5 };
   float mSpeedRandomize{ 0 };
   float mSpacingRandomize{ 1 };
   bool mOctaves{ false };
   float mWidth{ 1 };

private:
   void SpawnGrain(double time, double offset, float width, float speed);

   double mNextGrainSpawnMs{ 0 };
   int mNextGrainIdx{ 0 };
   Grain mGrains[MAX_GRAINS]{};
   bool mLiveMode{ false };
   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels]{};
};

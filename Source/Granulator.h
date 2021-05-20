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

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

#define MAX_GRAINS 16

class Grain
{
public:
   Grain() : mPos(0), mSpeed(0), mVol(0) {}
   void Spawn(double time, float pos, float speed, float lengthInMs, float vol);
   float Process(double time, const float* buffer, int bufferLength);
   void DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int bufferLength, bool wrapAround);
   void Clear() { mVol = 0; }
private:
   float GetWindow(double time);
   float mPos;
   float mSpeed;
   double mStartTime;
   double mEndTime;
   float mVol;
};

class Granulator
{
public:
   Granulator();
   float Process(double time, const float* buffer, int bufferLength, float offset);
   void Draw(float x, float y, float w, float h, int bufferStart, int bufferLength, bool wrapAround = true);
   void Reset();
   void ClearGrains();
   void SetLiveMode(bool live) { mLiveMode = live; }
   
   float mSpeed;
   float mGrainLengthMs;
   float mGrainSpacing;
   float mPosRandomizeMs;
   float mSpeedRandomize;
   float mSpacingRandomize;
   bool mOctaves;
   
private:
   void SpawnGrain(double time, float offset);
   
   double mLastGrainSpawnMs;
   int mNextGrainIdx;
   Grain mGrains[MAX_GRAINS];
   bool mLiveMode;
};

#endif /* defined(__modularSynth__Granulator__) */

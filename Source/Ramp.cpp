//
//  Ramp.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/26/12.
//
//

#include "Ramp.h"
#include "SynthGlobals.h"

void Ramp::Start(double end, double length)
{
   Start(gTime, Value(gTime), end, gTime+length);
}

void Ramp::Start(double curTime, float end, double endTime)
{
   Start(curTime, Value(curTime), end, endTime);
}

void Ramp::Start(double curTime, float start, float end, double endTime)
{
   mStartTime = curTime;
   mStartValue = start;
   mEndValue = end;
   mEndTime = endTime;
}

void Ramp::SetValue(float start)
{
   mStartValue = start;
   mEndValue = start;
   mStartTime = -1;
}

float Ramp::Value(double time) const
{
   if (mStartTime == -1 || time <= mStartTime)
      return mStartValue;
   if (time >= mEndTime)
      return mEndValue;
   
   double blend = (time - mStartTime) / (mEndTime - mStartTime);
   float retVal = mStartValue + blend * (mEndValue - mStartValue);
   if (fabsf(retVal) < FLT_EPSILON)
      return 0;
   return retVal;
}

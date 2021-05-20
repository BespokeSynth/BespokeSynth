//
//  Ramp.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/26/12.
//
//

#include "Ramp.h"
#include "SynthGlobals.h"

void Ramp::Start(double curTime, float end, double endTime)
{
   float startValue = Value(curTime);
   Start(curTime, startValue, end, endTime);
}

void Ramp::Start(double curTime, float start, float end, double endTime)
{
   mRampDatas[mRampDataPointer].mStartTime = curTime;
   mRampDatas[mRampDataPointer].mStartValue = start;
   mRampDatas[mRampDataPointer].mEndValue = end;
   mRampDatas[mRampDataPointer].mEndTime = endTime;
   mRampDataPointer = (mRampDataPointer + 1) % mRampDatas.size();
}

void Ramp::SetValue(float val)
{
   for (size_t i = 0; i < mRampDatas.size(); ++i)
   {
      mRampDatas[i].mStartValue = val;
      mRampDatas[i].mEndValue = val;
      mRampDatas[i].mStartTime = 0;
   }
}

bool Ramp::HasValue(double time) const
{
   const RampData* rampData = GetCurrentRampData(time);
   if (rampData->mStartTime == -1)
      return false;
   return true;
}

float Ramp::Value(double time) const
{
   const RampData* rampData = GetCurrentRampData(time);
   if (rampData->mStartTime == -1 || time <= rampData->mStartTime)
      return rampData->mStartValue;
   if (time >= rampData->mEndTime)
      return rampData->mEndValue;
   
   double blend = (time - rampData->mStartTime) / (rampData->mEndTime - rampData->mStartTime);
   float retVal = rampData->mStartValue + blend * (rampData->mEndValue - rampData->mStartValue);
   if (fabsf(retVal) < FLT_EPSILON)
      return 0;
   return retVal;
}

const Ramp::RampData* Ramp::GetCurrentRampData(double time) const
{
   int ret = 0;
   double latestTime = -1;
   for (int i = 0; i < (int)mRampDatas.size(); ++i)
   {
      if (mRampDatas[i].mStartTime < time && mRampDatas[i].mStartTime > latestTime)
      {
         ret = i;
         latestTime = mRampDatas[i].mStartTime;
      }
   }
   return &(mRampDatas[ret]);
}

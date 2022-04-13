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

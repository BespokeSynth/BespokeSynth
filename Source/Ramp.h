//
//  Ramp.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/26/12.
//
//

#ifndef __modularSynth__Ramp__
#define __modularSynth__Ramp__

#include <iostream>
#include <array>

class Ramp
{
public:
   Ramp() : mRampDataPointer(0) {}
   void Start(double curTime, float end, double endTime);
   void Start(double curTime, float start, float end, double endTime);
   void SetValue(float val);
   bool HasValue(double time) const;
   float Value(double time) const;
   float Target(double time) const { return GetCurrentRampData(time)->mEndValue; }
private:
   struct RampData
   {
      RampData() : mStartTime(-1), mStartValue(0), mEndValue(1), mEndTime(-1) {}
      double mStartTime;
      float mStartValue;
      float mEndValue;
      double mEndTime;
   };

   const RampData* GetCurrentRampData(double time) const;

   std::array<RampData, 10> mRampDatas;
   int mRampDataPointer;
};

#endif /* defined(__modularSynth__Ramp__) */

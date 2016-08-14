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

class Ramp
{
public:
   Ramp() : mStartTime(-1), mStartValue(0), mEndValue(1) {}
   void Start(double end, double length);
   void Start(double curTime, float end, double endTime);
   void Start(double curTime, float start, float end, double endTime);
   void SetValue(float start);
   float Value(double time) const;
   float Target() const { return mEndValue; }
private:
   double mStartTime;
   float mStartValue;
   float mEndValue;
   double mEndTime;
};

#endif /* defined(__modularSynth__Ramp__) */

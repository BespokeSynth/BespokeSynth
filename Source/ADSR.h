//
//  ADSR.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#ifndef __additiveSynth__ADSR__
#define __additiveSynth__ADSR__

#include <iostream>

class ADSR
{
public:
   ADSR() : mA(1), mD(.0001f), mS(1), mR(1), mOn(false), mStartTime(-10000), mStopTime(-10000), mTarget(0), mMaxSustain(0), mBlendFromValue(0) {}
   ADSR(float a, float d, float s, float r) : mA(a), mD(d), mS(s), mR(r), mOn(false), mStartTime(-10000), mStopTime(-10000), mMaxSustain(0), mBlendFromValue(0) {}
   void Start(double time, float target);
   void Start(double time, float target, float a, float d, float s, float r);
   void Start(double time, float target, ADSR adsr);
   void Stop(double time);
   float Value(double time);
   void Set(float a, float d, float s, float r, float h = 0);
   void Clear() { mTarget = 0; mOn = false; mStartTime = -10000; mStopTime = -10000;}
   void SetMaxSustain(float max) { mMaxSustain = max; }
   bool IsDone(double time);
   
   float mA;
   float mD;
   float mS;
   float mR;
   float mMaxSustain;
   
private:
   bool On() const;
   
   float mBlendFromValue;
   float mTarget;
   bool mOn;
   double mStartTime;
   double mStopTime;
};

#endif /* defined(__additiveSynth__ADSR__) */

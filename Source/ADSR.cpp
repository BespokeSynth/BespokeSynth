//
//  ADSR.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#include "ADSR.h"
#include "OpenFrameworksPort.h"

void ADSR::Set(float a, float d, float s, float r, float h /*=0*/)
{
   mA = MAX(a,1);
   mD = MAX(d,.0001f);
   mS = MAX(s,.0001f);
   mR = MAX(r,1);
   mMaxSustain = h;
}

void ADSR::Start(double time, float target, float a, float d, float s, float r)
{
   Set(a,d,s,r);
   Start(time, target);
}

void ADSR::Start(double time, float target, ADSR adsr)
{
   Set(adsr.mA,adsr.mD,adsr.mS,adsr.mR,adsr.mMaxSustain);
   Start(time, target);
}

void ADSR::Start(double time, float target)
{
   mBlendFromValue = Value(time);
   mStartTime = time;
   mTarget = target;
   mOn = true;
}

void ADSR::Stop(double time)
{
   mTarget = Value(time);
   mStopTime = time;
   mOn = false;
}

float ADSR::Value(double time)
{
   if (mMaxSustain != 0)
   {
      if (mOn && //check max sustain
          time >= mStartTime + mMaxSustain + mA + mD)
      {
         //stop
         mTarget = mS * mTarget;
         mStopTime = time;
         mOn = false;
      }
   }
   
   float blend = ofClamp(1 - (time - mStartTime) / mR, 0, 1) * mBlendFromValue;
   
   if (On()) //attack/sustain
   {
      time -= mStartTime;
      if (time <= mA)
         return time / mA * mTarget + blend;
      if (time <= mA + mD)
         return (1.0f - (1-mS) * ((time - mA) / mD)) * mTarget + blend;
      return mS * mTarget + blend;
   }
   else //release
   {
      time -= mStopTime;
      if (time > mR)
         return 0 + blend;
      return (1 - (time / mR)) * mTarget + blend;
   }
}

bool ADSR::IsDone(double time)
{
   if (On())
      return false;
   
   time -= mStopTime;
   if (time > mR)
      return true;
   
   return false;
}

bool ADSR::On() const
{
   return mOn;
}
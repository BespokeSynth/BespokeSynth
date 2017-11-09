//
//  ADSR.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#include "ADSR.h"
#include "OpenFrameworksPort.h"

void ADSR::Set(float a, float d, float s, float r, float h /*=-1*/)
{
   mStages[0].target = 1;
   mStages[0].time = MAX(a,1);
   mStages[1].target = s;
   mStages[1].time = MAX(d,1);
   mStages[1].target = MAX(s,.0001f);
   mStages[2].target = 0;
   mStages[2].time = MAX(r,1);
   mNumStages = 3;
   mSustainStage = 1;
   mMaxSustain = h;
}

void ADSR::Set(const ADSR& other)
{
   for (int i=0; i<other.mNumStages; ++i)
      mStages[i] = other.mStages[i];
   mNumStages = other.mNumStages;
   mSustainStage = other.mSustainStage;
}

void ADSR::Start(double time, float target, float a, float d, float s, float r)
{
   Set(a,d,s,r);
   Start(time, target);
}

void ADSR::Start(double time, float target, const ADSR& adsr)
{
   Set(adsr);
   Start(time, target);
}

void ADSR::Start(double time, float target)
{
   mBlendFromValue = Value(time);
   mEventTime = time;
   mMult = target;
   mOn = true;
}

void ADSR::Stop(double time)
{
   mBlendFromValue = Value(time);
   mEventTime = time;
   mOn = false;
}

float ADSR::Value(double time) const
{
   float stageStartValue;
   double stageStartTime;
   int stage = GetStage(time, stageStartTime);
   if (stage == mNumStages)  //done
      return 0;
   
   if (stage == 0 || stage == mSustainStage + 1)
      stageStartValue = mBlendFromValue;
   else
      stageStartValue = mStages[stage-1].target * mMult;
   
   if (stage == mSustainStage && time > stageStartTime + mStages[mSustainStage].time)
      return mStages[mSustainStage].target * mMult;
   return ofLerp(stageStartValue, mStages[stage].target * mMult, (time - stageStartTime) / mStages[stage].time);
}

int ADSR::GetStage(double time, double& stageStartTimeOut) const
{
   int stage;
   if (mOn)
      stage = 0;
   else
      stage = mSustainStage+1;
   stageStartTimeOut = mEventTime;
   while (time > mStages[stage].time + stageStartTimeOut && stage < mNumStages)
   {
      stageStartTimeOut += mStages[stage].time;
      ++stage;
      if (stage == mSustainStage)
      {
         if (mMaxSustain == -1)
            break;
         else
            stageStartTimeOut += mMaxSustain;
      }
   }
   
   return stage;
}

bool ADSR::IsDone(double time) const
{
   double dummy;
   return GetStage(time, dummy) == mNumStages;
}

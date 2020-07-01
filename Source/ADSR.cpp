//
//  ADSR.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#include "ADSR.h"
#include "OpenFrameworksPort.h"
#include "MathUtils.h"
#include "FileStream.h"
#include "SynthGlobals.h"
#include "Profiler.h"

void ::ADSR::Set(float a, float d, float s, float r, float h /*=-1*/)
{
   mStages[0].target = 1;
   mStages[0].time = MAX(a,1);
   mStages[0].curve = 0;
   mStages[1].target = MAX(s,.0001f);
   mStages[1].time = MAX(d,1);
   mStages[0].curve = 0;
   mStages[2].target = 0;
   mStages[2].time = MAX(r,1);
   mStages[0].curve = 0;
   mNumStages = 3;
   mSustainStage = 1;
   mMaxSustain = h;
   mHasSustainStage = true;
}

void ::ADSR::Set(const ADSR& other)
{
   for (int i=0; i<other.mNumStages; ++i)
      mStages[i] = other.mStages[i];
   mNumStages = other.mNumStages;
   mSustainStage = other.mSustainStage;
   mMaxSustain = other.mMaxSustain;
   mHasSustainStage = other.mHasSustainStage;
   mFreeReleaseLevel = other.mFreeReleaseLevel;
}

void ::ADSR::Start(double time, float target, float a, float d, float s, float r)
{
   Set(a,d,s,r);
   Start(time, target);
}

void ::ADSR::Start(double time, float target, const ADSR& adsr)
{
   Set(adsr);
   Start(time, target);
}

void ::ADSR::Start(double time, float target)
{
   mStartBlendFromValue = Value(time);
   mStartTime = time;
   mMult = target;
   
   if (mMaxSustain != -1 && mHasSustainStage)
   {
      float stopTime = time;
      for (int i=0; i<mNumStages; ++i)
      {
         stopTime += mStages[i].time;
         if (i == mSustainStage)
            break;
      }
      stopTime += mMaxSustain;
      Stop(stopTime);
   }
}

void ::ADSR::Stop(double time, bool warn /*= true*/)
{
   mStopBlendFromValue = Value(time);
   if (time <= mStartTime)
   {
      if (warn)
         ofLog() << "trying to stop before we started (" << time << "<=" << mStartTime << ")";
      time = mStartTime + .0001f;  //must be after start
   }
   mStopTime = time;
}

float ::ADSR::Value(double time) const
{
   //if (mStartTime < 0)
   //   return 0;
   
   //PROFILER(ADSR_Value);
   
   float stageStartValue;
   double stageStartTime;
   int stage = GetStage(time, stageStartTime);
   if (stage == mNumStages)  //done
      return mStages[stage-1].target;
   
   if (stage == 0)
      stageStartValue = mStartBlendFromValue;
   else if (mHasSustainStage && stage == mSustainStage + 1)
      stageStartValue = mStopBlendFromValue;
   else
      stageStartValue = mStages[stage-1].target * mMult;
   
   if (mHasSustainStage && stage == mSustainStage && time > stageStartTime + mStages[mSustainStage].time)
      return mStages[mSustainStage].target * mMult;
   
   float lerp = (time - stageStartTime) / mStages[stage].time;
   if (mStages[stage].curve != 0)
      lerp = MathUtils::Curve(lerp, mStages[stage].curve * ((stageStartValue < mStages[stage].target*mMult) ? 1 : -1));
   
   return ofLerp(stageStartValue, mStages[stage].target * mMult, lerp);
}

int ::ADSR::GetStage(double time, double& stageStartTimeOut) const
{
   if (mStartTime < 0)
      return mNumStages;
   
   int stage = 0;
   stageStartTimeOut = mStartTime;
   
   if (time >= mStartTime)
   {
      if (mHasSustainStage && time >= mStopTime && mStopTime > mStartTime)
      {
         stage = mSustainStage+1;
         stageStartTimeOut = mStopTime;
      }
      
      while (time > mStages[stage].time + stageStartTimeOut && stage < mNumStages)
      {
         stageStartTimeOut += mStages[stage].time;
         ++stage;
         if (mHasSustainStage && stage == mSustainStage)
            break;
      }
   }
   
   return stage;
}

bool ::ADSR::IsDone(double time) const
{
   double dummy;
   return GetStage(time, dummy) == mNumStages;
}

int ::ADSR::GetStageForTime(double time) const
{
   double dummy;
   return GetStage(time, dummy);
}

namespace
{
   const int kSaveStateRev = 0;
}

void ::ADSR::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mMult;
   out << mSustainStage;
   out << mMaxSustain;
   out << mNumStages;
   out << mHasSustainStage;
   out << mFreeReleaseLevel;
   out << MAX_ADSR_STAGES;
   for (int i=0; i<MAX_ADSR_STAGES; ++i)
   {
      out << mStages[i].curve;
      out << mStages[i].target;
      out << mStages[i].time;
   }
}

void ::ADSR::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   in >> mMult;
   in >> mSustainStage;
   in >> mMaxSustain;
   in >> mNumStages;
   in >> mHasSustainStage;
   in >> mFreeReleaseLevel;
   int maxNumStages;
   in >> maxNumStages;
   assert(maxNumStages == MAX_ADSR_STAGES);
   for (int i=0; i<maxNumStages; ++i)
   {
      in >> mStages[i].curve;
      in >> mStages[i].target;
      in >> mStages[i].time;
   }
}

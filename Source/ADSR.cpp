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

void ::ADSR::Set(float a, float d, float s, float r, float h /*=-1*/)
{
   mStages[0].target = 1;
   mStages[0].time = MAX(a, 1);
   mStages[0].curve = 0;
   mStages[1].target = MAX(s, .0001f);
   mStages[1].time = MAX(d, 1);
   mStages[1].curve = -.5f;
   mStages[2].target = 0;
   mStages[2].time = MAX(r, 1);
   mStages[2].curve = -.5f;
   mNumStages = 3;
   mSustainStage = 1;
   mMaxSustain = h;
   mHasSustainStage = true;
}

void ::ADSR::Set(const ADSR& other)
{
   for (int i = 0; i < other.mNumStages; ++i)
      mStages[i] = other.mStages[i];
   mNumStages = other.mNumStages;
   mSustainStage = other.mSustainStage;
   mMaxSustain = other.mMaxSustain;
   mHasSustainStage = other.mHasSustainStage;
   mFreeReleaseLevel = other.mFreeReleaseLevel;
}

void ::ADSR::Start(double time, float target, float a, float d, float s, float r, float timeScale /*=1*/, float curve /*=0*/)
{
   Set(a, d, s, r);
   Start(time, target, timeScale, curve);
}

void ::ADSR::Start(double time, float target, const ADSR& adsr, float timeScale /*=1*/, float curve /*=0*/)
{
   Set(adsr);
   Start(time, target, timeScale, curve);
}

void ::ADSR::Start(double time, float target, float timeScale /*=1*/, float curve /*=0*/)
{
   mEvents[mNextEventPointer].Reset();
   mEvents[mNextEventPointer].mStartBlendFromValue = Value(time);
   mEvents[mNextEventPointer].mStartTime = time;
   mEvents[mNextEventPointer].mMult = target;
   mNextEventPointer = (mNextEventPointer + 1) % mEvents.size();
   mTimeScale = timeScale;
   mCurve = curve;

   if (mMaxSustain >= 0 && mHasSustainStage)
   {
      float stopTime = time;
      for (int i = 0; i < mNumStages; ++i)
      {
         stopTime += mStages[i].time * timeScale;
         if (i == mSustainStage)
            break;
      }
      stopTime += mMaxSustain;
      Stop(stopTime);
   }
}

void ::ADSR::Stop(double time, bool warn /*= true*/)
{
   EventInfo* e = GetEvent(time);

   e->mStopBlendFromValue = Value(time);

   if (time <= e->mStartTime)
   {
      if (warn)
         ofLog() << "trying to stop before we started (" << time << "<=" << e->mStartTime << ")";
      time = e->mStartTime + .0001f; //must be after start
   }
   e->mStopTime = time;
}

::ADSR::EventInfo* ::ADSR::GetEvent(double time)
{
   int ret = 0;
   double latestTime = -1;
   for (int i = 0; i < (int)mEvents.size(); ++i)
   {
      if (mEvents[i].mStartTime < time && mEvents[i].mStartTime > latestTime)
      {
         ret = i;
         latestTime = mEvents[i].mStartTime;
      }
   }
   return &(mEvents[ret]);
}

const ::ADSR::EventInfo* ::ADSR::GetEventConst(double time) const
{
   int ret = 0;
   double latestTime = -1;
   for (int i = 0; i < (int)mEvents.size(); ++i)
   {
      if (mEvents[i].mStartTime < time && mEvents[i].mStartTime > latestTime)
      {
         ret = i;
         latestTime = mEvents[i].mStartTime;
      }
   }
   return &(mEvents[ret]);
}

float ::ADSR::Value(double time) const
{
   const EventInfo* e = GetEventConst(time);
   return Value(time, e);
}

float ::ADSR::Value(double time, const EventInfo* e) const
{
   float stageStartValue;
   double stageStartTime;
   int stage = GetStage(time, stageStartTime, e);
   if (stage == mNumStages) //done
      return mStages[stage - 1].target;

   if (stage == 0)
      stageStartValue = e->mStartBlendFromValue;
   else if (mHasSustainStage && stage == mSustainStage + 1 && e->mStopBlendFromValue != std::numeric_limits<float>::max())
      stageStartValue = e->mStopBlendFromValue;
   else
      stageStartValue = mStages[stage - 1].target * e->mMult;

   if (mHasSustainStage && stage == mSustainStage && time > stageStartTime + (mStages[mSustainStage].time * GetStageTimeScale(mSustainStage)))
      return mStages[mSustainStage].target * e->mMult;

   float stageTimeScale = GetStageTimeScale(stage);

   float lerp = ofClamp((time - stageStartTime) / (mStages[stage].time * stageTimeScale), 0, 1);
   float curve = mStages[stage].curve + mCurve;
   if (curve != 0)
      lerp = MathUtils::Curve(lerp, curve * ((stageStartValue < mStages[stage].target * e->mMult) ? 1 : -1));

   return ofLerp(stageStartValue, mStages[stage].target * e->mMult, lerp);
}

float ::ADSR::GetStageTimeScale(int stage) const
{
   if (stage >= mNumStages - 1)
      return 1;
   return mTimeScale;
}

int ::ADSR::GetStage(double time, double& stageStartTimeOut) const
{
   const EventInfo* e = GetEventConst(time);
   return GetStage(time, stageStartTimeOut, e);
}

int ::ADSR::GetStage(double time, double& stageStartTimeOut, const EventInfo* e) const
{
   if (e->mStartTime < 0)
      return mNumStages;

   int stage = 0;
   stageStartTimeOut = e->mStartTime;

   if (time >= e->mStartTime)
   {
      if (mHasSustainStage && time >= e->mStopTime && e->mStopTime > e->mStartTime)
      {
         stage = mSustainStage + 1;
         stageStartTimeOut = e->mStopTime;
      }

      while (time > mStages[stage].time * GetStageTimeScale(stage) + stageStartTimeOut && stage < mNumStages)
      {
         stageStartTimeOut += mStages[stage].time * GetStageTimeScale(stage);
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

namespace
{
   const int kSaveStateRev = 1;
}

void ::ADSR::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   float dummy;
   out << dummy;
   out << mSustainStage;
   out << mMaxSustain;
   out << mNumStages;
   out << mHasSustainStage;
   out << mFreeReleaseLevel;
   out << MAX_ADSR_STAGES;
   for (int i = 0; i < MAX_ADSR_STAGES; ++i)
   {
      out << mStages[i].curve;
      out << mStages[i].target;
      out << mStages[i].time;
   }
   out << mTimeScale;
}

void ::ADSR::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   float dummy;
   in >> dummy;
   in >> mSustainStage;
   in >> mMaxSustain;
   in >> mNumStages;
   in >> mHasSustainStage;
   in >> mFreeReleaseLevel;
   int maxNumStages;
   in >> maxNumStages;
   assert(maxNumStages == MAX_ADSR_STAGES);
   for (int i = 0; i < maxNumStages; ++i)
   {
      in >> mStages[i].curve;
      in >> mStages[i].target;
      in >> mStages[i].time;
   }

   if (rev >= 1)
      in >> mTimeScale;
}

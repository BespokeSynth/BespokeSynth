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
//  ADSR.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#pragma once

#include <vector>
#include <array>
#include "OpenFrameworksPort.h"

#define MAX_ADSR_STAGES 20

class FileStreamOut;
class FileStreamIn;

class ADSR
{
public:
   struct Stage
   {
      float target{ 0 };
      float time{ 1 };
      float curve{ 0 };
   };

   struct EventInfo
   {
      EventInfo(){};
      EventInfo(double startTime, double stopTime)
      {
         mStartBlendFromValue = 0;
         mStopBlendFromValue = std::numeric_limits<float>::max();
         mMult = 1;
         mStartTime = startTime;
         mStopTime = stopTime;
      }
      void Reset()
      {
         mStartBlendFromValue = 0;
         mStopBlendFromValue = 0;
         mMult = 1;
         mStartTime = -10000;
         mStopTime = -10000;
      }
      float mStartBlendFromValue{ 0 };
      float mStopBlendFromValue{ 0 };
      float mMult{ 1 };
      double mStartTime{ -10000 };
      double mStopTime{ -10000 };
   };

   ADSR(float a, float d, float s, float r)
   {
      Set(a, d, s, r);
   }
   ADSR()
   : ADSR(1, 1, 1, 1)
   {}
   void Start(double time, float target, float timeScale = 1, float curve = 0);
   void Start(double time, float target, float a, float d, float s, float r, float timeScale = 1, float curve = 0);
   void Start(double time, float target, const ADSR& adsr, float timeScale = 1, float curve = 0);
   void Stop(double time, bool warn = true);
   float Value(double time) const;
   float Value(double time, const EventInfo* event) const;
   void Set(float a, float d, float s, float r, float h = -1);
   void Set(const ADSR& other);
   void Clear()
   {
      for (auto& e : mEvents)
      {
         e.Reset();
      }
   }
   void SetMaxSustain(float max) { mMaxSustain = max; }
   void SetSustainStage(int stage) { mSustainStage = stage; }
   bool IsDone(double time) const;
   bool IsStandardADSR() const { return mNumStages == 3 && mSustainStage == 1; }
   float GetStartTime(double time) const { return GetEventConst(time)->mStartTime; }
   float GetStopTime(double time) const { return GetEventConst(time)->mStopTime; }

   int GetNumStages() const { return mNumStages; }
   void SetNumStages(int num) { mNumStages = CLAMP(num, 1, MAX_ADSR_STAGES); }
   Stage& GetStageData(int stage) { return mStages[stage]; }
   int GetStage(double time, double& stageStartTimeOut) const;
   int GetStage(double time, double& stageStartTimeOut, const EventInfo* e) const;

   float GetTimeScale() const { return mTimeScale; }

   float& GetA() { return mStages[0].time; }
   float& GetD() { return mStages[1].time; }
   float& GetS() { return mStages[1].target; }
   float& GetR() { return mStages[2].time; }
   float& GetMaxSustain() { return mMaxSustain; }
   int& GetSustainStage() { return mSustainStage; }
   bool& GetHasSustainStage() { return mHasSustainStage; }
   bool& GetFreeReleaseLevel() { return mFreeReleaseLevel; }

   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   EventInfo* GetEvent(double time);
   const EventInfo* GetEventConst(double time) const;
   float GetStageTimeScale(int stage) const;

   std::array<EventInfo, 5> mEvents;
   int mNextEventPointer{ 0 };
   int mSustainStage{ 0 };
   float mMaxSustain{ -1 };
   Stage mStages[MAX_ADSR_STAGES];
   int mNumStages{ 0 };
   bool mHasSustainStage{ false };
   bool mFreeReleaseLevel{ false };
   float mTimeScale{ 1 };
   float mCurve{ 0 };
};

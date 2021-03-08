//
//  ADSR.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#pragma once

#include <iostream>
#include <vector>
#include <array>

#define MAX_ADSR_STAGES 20

class FileStreamOut;
class FileStreamIn;

class ADSR
{
public:
   struct Stage
   {
      Stage() : target(0), time(1), curve(0) {}
      float target;
      float time;
      float curve;
   };
   
   ADSR(float a, float d, float s, float r) : mNextEventPointer(0), mMaxSustain(-1), mFreeReleaseLevel(false), mTimeScale(1) { Set(a,d,s,r); }
   ADSR() : ADSR(1,1,1,1) {}
   void Start(double time, float target, float timeScale = 1);
   void Start(double time, float target, float a, float d, float s, float r, float timeScale = 1);
   void Start(double time, float target, const ADSR& adsr, float timeScale = 1);
   void Stop(double time, bool warn = true);
   float Value(double time) const;
   void Set(float a, float d, float s, float r, float h = -1);
   void Set(const ADSR& other);
   void Clear() { for (auto& e : mEvents) { e.Reset(); } }
   void SetMaxSustain(float max) { mMaxSustain = max; }
   void SetSustainStage(int stage) { mSustainStage = stage; }
   bool IsDone(double time) const;
   bool IsStandardADSR() const { return mNumStages == 3 && mSustainStage == 1; }
   float GetStartTime(double time) const { return GetEventConst(time)->mStartTime; }
   float GetStopTime(double time) const { return GetEventConst(time)->mStopTime; }
   
   int GetNumStages() const { return mNumStages; }
   void SetNumStages(int num) { mNumStages = num; }
   Stage& GetStageData(int stage) { return mStages[stage]; }
   int GetStageForTime(double time) const;
   int GetStage(double time, double& stageStartTimeOut) const;

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
   struct EventInfo
   {
      EventInfo() { Reset(); }
      void Reset() { mStartBlendFromValue = 0; mStopBlendFromValue = 0; mMult = 1; mStartTime = -10000; mStopTime = -10000; }
      float mStartBlendFromValue;
      float mStopBlendFromValue;
      float mMult;
      double mStartTime;
      double mStopTime;
   };

   EventInfo* GetEvent(double time);
   const EventInfo* GetEventConst(double time) const;
   
   std::array<EventInfo, 5> mEvents;
   int mNextEventPointer;
   int mSustainStage;
   float mMaxSustain;
   Stage mStages[MAX_ADSR_STAGES];
   int mNumStages;
   bool mHasSustainStage;
   bool mFreeReleaseLevel;
   float mTimeScale;
};

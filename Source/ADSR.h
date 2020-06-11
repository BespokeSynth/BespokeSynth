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
#include <vector>

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
   
   ADSR(float a, float d, float s, float r) : mStartBlendFromValue(0), mStopBlendFromValue(0), mStartTime(-10000), mStopTime(-10000), mMaxSustain(-1), mFreeReleaseLevel(false) { Set(a,d,s,r); }
   ADSR() : ADSR(1,1,1,1) {}
   void Start(double time, float target);
   void Start(double time, float target, float a, float d, float s, float r);
   void Start(double time, float target, const ADSR& adsr);
   void Stop(double time, bool warn = true);
   float Value(double time) const;
   void Set(float a, float d, float s, float r, float h = -1);
   void Set(const ADSR& other);
   void Clear() { mMult = 0; mStartTime = -10000; mStopTime = -10000; mStartBlendFromValue = 0; mStopBlendFromValue = 0; }
   void SetMaxSustain(float max) { mMaxSustain = max; }
   void SetSustainStage(int stage) { mSustainStage = stage; }
   bool IsDone(double time) const;
   bool IsStandardADSR() const { return mNumStages == 3 && mSustainStage == 1; }
   float GetStartTime() const { return mStartTime; }
   float GetStopTime() const { return mStopTime; }
   
   int GetNumStages() const { return mNumStages; }
   void SetNumStages(int num) { mNumStages = num; }
   Stage& GetStageData(int stage) { return mStages[stage]; }
   int GetStageForTime(double time) const;
   int GetStage(double time, double& stageStartTimeOut) const;
   
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
   float mStartBlendFromValue;
   float mStopBlendFromValue;
   float mMult;
   double mStartTime;
   double mStopTime;
   int mSustainStage;
   float mMaxSustain;
   Stage mStages[MAX_ADSR_STAGES];
   int mNumStages;
   bool mHasSustainStage;
   bool mFreeReleaseLevel;
};

#endif /* defined(__additiveSynth__ADSR__) */

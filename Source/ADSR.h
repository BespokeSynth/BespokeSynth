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

class ADSR
{
public:
   struct Stage
   {
      Stage() : target(0), time(1) {}
      float target;
      float time;
   };
   
   ADSR(float a, float d, float s, float r) : mOn(false), mEventTime(-10000), mBlendFromValue(0), mMaxSustain(-1) { Set(a,d,s,r); }
   ADSR() : ADSR(1,1,1,1) {}
   void Start(double time, float target);
   void Start(double time, float target, float a, float d, float s, float r);
   void Start(double time, float target, const ADSR& adsr);
   void Stop(double time);
   float Value(double time) const;
   void Set(float a, float d, float s, float r, float h = -1);
   void Set(const ADSR& other);
   void Clear() { mOn = false; mMult = 0; mEventTime = -10000;}
   void SetMaxSustain(float max) { mMaxSustain = max; }
   bool IsDone(double time) const;
   bool IsStandardADSR() const { return mNumStages == 3 && mSustainStage == 1; }
   
   float& GetA() { return mStages[0].time; }
   float& GetD() { return mStages[1].time; }
   float& GetS() { return mStages[1].target; }
   float& GetR() { return mStages[2].time; }
   
private:
   int GetStage(double time, double& stageStartTimeOut) const;
   
   bool mOn;
   float mBlendFromValue;
   float mMult;
   double mEventTime;
   int mSustainStage;
   float mMaxSustain;
   Stage mStages[MAX_ADSR_STAGES];
   int mNumStages;
};

#endif /* defined(__additiveSynth__ADSR__) */

//
//  Profiler.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#ifndef __modularSynth__Profiler__
#define __modularSynth__Profiler__

#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"

#define PROFILER_HISTORY_LENGTH 500
#define PROFILER_MAX_TRACK 100

#define PROFILER(profile_id) static uint32_t profile_id ## _hash = JenkinsHash(#profile_id); Profiler profilerScopeHolder(#profile_id, profile_id ## _hash)

class Profiler
{
public:
   Profiler(const char* name, uint32_t hash);
   ~Profiler();
   
   static void PrintCounters();
   static void Draw();
   
   static void ToggleProfiler();
   
private:
   static long GetSafeFrameLengthNanoseconds();
   
   struct Cost
   {
      Cost() : mFrameCost(0), mHistoryIdx(0) { bzero(mHistory, sizeof(long)); }
      void EndFrame();
      unsigned long long MaxCost() const;
      
      string mName;
      uint32_t mHash;
      unsigned long long mFrameCost;
      unsigned long long mHistory[PROFILER_HISTORY_LENGTH];
      int mHistoryIdx;
   };
   
   unsigned long long mTimerStart;
   int mIndex;
   
   static Cost sCosts[PROFILER_MAX_TRACK];
   static bool sEnableProfiler;
};

#endif /* defined(__modularSynth__Profiler__) */

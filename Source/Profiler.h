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

class Profiler
{
public:
   Profiler(const char* name, bool master = false);
   ~Profiler();
   
   static void PrintCounters();
   static void Draw();
   
   static float GetUsage(const char* counter);
   
   static void ToggleProfiler() { sEnableProfiler = !sEnableProfiler; sCosts.clear(); }
   
private:
   static long GetSafeFrameLengthMicroseconds();
   
   struct Cost
   {
      Cost() : mFrameCost(0), mHistoryIdx(0) { bzero(mHistory, sizeof(long)); }
      void EndFrame();
      long MaxCost() const;
      
      long mFrameCost;
      long mHistory[PROFILER_HISTORY_LENGTH];
      int mHistoryIdx;
   };
   
   long mTimerStart;
   uint32_t mName;
   bool mMaster;
   
   static map<uint32_t, string> sNameLookup;
   static map<uint32_t, Cost> sCosts;
   static bool sEnableProfiler;
};

#endif /* defined(__modularSynth__Profiler__) */

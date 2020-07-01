//
//  Profiler.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#include "Profiler.h"
#include "SynthGlobals.h"
#include <time.h>

Profiler::Cost Profiler::sCosts[];
bool Profiler::sEnableProfiler = false;

namespace {
   static inline uint64_t rdtscp( uint32_t & aux )
   {
#if BESPOKE_WINDOWS
      return 0;   //TODO(Ryan)
#else
      uint64_t rax,rdx;
      asm volatile ( "rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : : );
      return (rdx << 32) + rax;
#endif
   }
}

Profiler::Profiler(const char* name, uint32_t hash)
: mIndex(-1)
{
   if (sEnableProfiler)
   {
      for (int i=0; i<PROFILER_MAX_TRACK; ++i)
      {
         if (sCosts[i].mHash == hash)
         {
            mIndex = i;
            break;
         }
         if (sCosts[i].mName[0] == 0)
         {
            mIndex = i;
            sCosts[i].mName = name;
            sCosts[i].mHash = hash;
            break;
         }
      }
      
      uint32_t aux;
      mTimerStart = rdtscp(aux);

      //struct timespec t;
      //clock_gettime(CLOCK_MONOTONIC, &t);
      //mTimerStart = t.tv_sec * 1000000000 + t.tv_nsec;
   }
}

Profiler::~Profiler()
{
   if (sEnableProfiler)
   {
      uint32_t aux;
      sCosts[mIndex].mFrameCost += rdtscp(aux) - mTimerStart;
      
      //struct timespec t;
      //clock_gettime(CLOCK_MONOTONIC, &t);
      //unsigned long long timerEnd = t.tv_sec * 1000000000 + t.tv_nsec;
   
      //sCosts[mIndex].mFrameCost += timerEnd - mTimerStart;
   }
}

//static
void Profiler::PrintCounters()
{
   //bool printedBreak = false;
   for (int i=0; i<PROFILER_MAX_TRACK; ++i)
   {
      if (sCosts[i].mName[0] == 0)
         break;
      /*if (sCosts[i].mFrameCost > 500)
      {
         if (!printedBreak)
         {
            ofLog() << "******* PrintCounters():";
            printedBreak = true;
         }
         ofLog() << sCosts[i].mName << " " << sCosts[i].mFrameCost << " us";
      }*/
      sCosts[i].EndFrame();
   }
}

//static
void Profiler::Draw()
{
   if (!sEnableProfiler)
      return;
   
   ofPushMatrix();
   ofTranslate(30,70);
   ofPushStyle();
   ofFill();
   ofSetColor(0,0,0,140);
   //ofRect(-5,-15,600,sCosts.size()*15+10);
   long entireFrameUs = GetSafeFrameLengthNanoseconds();
   for (int i=0; i<PROFILER_MAX_TRACK; ++i)
   {
      if (sCosts[i].mName[0] == 0)
         break;
      const Cost& cost = sCosts[i];
      long maxCost = cost.MaxCost();
      
      ofSetColor(255,255,255);
      gFont.DrawString(string(sCosts[i].mName)+": "+ofToString(maxCost/1000), 15, 0, 0);
      
      if (maxCost > entireFrameUs)
         ofSetColor(255,0,0);
      else
         ofSetColor(0,255,0);
      ofRect(250, -10,(float)maxCost / entireFrameUs * (ofGetWidth() - 300) * .5f, 10);
      
      ofTranslate(0, 15);
   }
   ofPopStyle();
   ofPopMatrix();
}

//static
long Profiler::GetSafeFrameLengthNanoseconds()
{
   //using about 70% of the length of buffer size doing processing seems to be safe
   //for avoiding starvation issues
   return long((gBufferSize / float(gSampleRate)) * 1000000000 * .7f);
}

//static
void Profiler::ToggleProfiler()
{
   sEnableProfiler = !sEnableProfiler;
   
   for (int i=0; i<PROFILER_MAX_TRACK; ++i)
      sCosts[i].mName[0] = 0;
}

void Profiler::Cost::EndFrame()
{
   mHistory[mHistoryIdx] = mFrameCost;
   mFrameCost = 0;
   ++mHistoryIdx;
   if (mHistoryIdx >= PROFILER_HISTORY_LENGTH)
      mHistoryIdx = 0;
}

unsigned long long Profiler::Cost::MaxCost() const
{
   unsigned long long maxCost = 0;
   for (int i=0; i<PROFILER_HISTORY_LENGTH; ++i)
      maxCost = MAX(maxCost, mHistory[i]);
   return maxCost;
}

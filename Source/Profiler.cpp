//
//  Profiler.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#include "Profiler.h"
#include "SynthGlobals.h"

map<uint32_t, string> Profiler::sNameLookup;
map<uint32_t, Profiler::Cost> Profiler::sCosts;
bool Profiler::sEnableProfiler = false;

Profiler::Profiler(const char* name, bool master)
: mMaster(master)
{
   if (sEnableProfiler || mMaster)
   {
      mName = JenkinsHash(name);
      mTimerStart = ofGetSystemTimeMicros();
      if (sNameLookup.find(mName) == sNameLookup.end())
         sNameLookup[mName] = name;
   }
}

Profiler::~Profiler()
{
   if (sEnableProfiler || mMaster)
   {
      long timerEnd = ofGetSystemTimeMicros();
      long timerLength = timerEnd - mTimerStart;
   
      sCosts[mName].mFrameCost += timerLength;
   }
}

//static
void Profiler::PrintCounters()
{
   //bool printedBreak = false;
   for (auto iter = sCosts.begin(); iter != sCosts.end(); ++iter)
   {
      /*if ((*iter).second.mFrameCost > 500)
      {
         if (!printedBreak)
         {
            ofLog() << "******* PrintCounters():";
            printedBreak = true;
         }
         ofLog() << (*iter).first << " " << (*iter).second.mFrameCost << " us";
      }*/
      (*iter).second.EndFrame();
   }
}

//static
void Profiler::Draw()
{
   if (!sEnableProfiler)
      return;
   
   ofPushMatrix();
   ofTranslate(30,30);
   ofPushStyle();
   ofFill();
   ofSetColor(0,0,0,140);
   ofRect(-5,-15,600,sCosts.size()*15+10);
   long entireFrameUs = GetSafeFrameLengthMicroseconds();
   for (auto iter = sCosts.begin(); iter != sCosts.end(); ++iter)
   {
      const Cost& cost = (*iter).second;
      long maxCost = cost.MaxCost();
      
      ofSetColor(255,255,255);
      gFont.DrawString(sNameLookup[(*iter).first]+": "+ofToString(maxCost), 15, 0, 0);
      
      if (maxCost > entireFrameUs)
         ofSetColor(255,0,0);
      else
         ofSetColor(0,255,0);
      ofRect(250, -10, maxCost, 10);
      
      ofTranslate(0, 15);
   }
   ofPopStyle();
   ofPopMatrix();
}

//static
long Profiler::GetSafeFrameLengthMicroseconds()
{
   //using about 70% of the length of buffer size doing processing seems to be safe
   //for avoiding starvation issues
   return long((gBufferSize / float(gSampleRate)) * 1000 * 1000 * .7f);
}

//static
float Profiler::GetUsage(const char* counter)
{
   return sCosts[JenkinsHash(counter)].MaxCost() / float(GetSafeFrameLengthMicroseconds());
}

void Profiler::Cost::EndFrame()
{
   mHistory[mHistoryIdx] = mFrameCost;
   mFrameCost = 0;
   ++mHistoryIdx;
   if (mHistoryIdx >= PROFILER_HISTORY_LENGTH)
      mHistoryIdx = 0;
}

long Profiler::Cost::MaxCost() const
{
   long maxCost = 0;
   for (int i=0; i<PROFILER_HISTORY_LENGTH; ++i)
      maxCost = MAX(maxCost, mHistory[i]);
   return maxCost;
}

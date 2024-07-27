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
//  Profiler.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/28/14.
//
//

#pragma once

#include "OpenFrameworksPort.h"

#define PROFILER_HISTORY_LENGTH 500
#define PROFILER_MAX_TRACK 100

#define PROFILER(profile_id)                                     \
   static uint32_t profile_id##_hash = JenkinsHash(#profile_id); \
   Profiler profilerScopeHolder(#profile_id, profile_id##_hash)

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
      void EndFrame();
      unsigned long long MaxCost() const;

      std::string mName;
      uint32_t mHash{ 0 };
      unsigned long long mFrameCost{ 0 };
      unsigned long long mHistory[PROFILER_HISTORY_LENGTH]{};
      int mHistoryIdx{ 0 };
   };

   unsigned long long mTimerStart{ 0 };
   int mIndex{ -1 };

   static Cost sCosts[PROFILER_MAX_TRACK];
   static bool sEnableProfiler;
};

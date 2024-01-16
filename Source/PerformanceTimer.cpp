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
//  PerformanceTimer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/26/15.
//
//

#include "PerformanceTimer.h"
#include "SynthGlobals.h"

TimerInstance::TimerInstance(std::string name, PerformanceTimer& manager)
: mName(name)
, mManager(manager)
{
   mTimerStart = ofGetSystemTimeNanos();
}

TimerInstance::~TimerInstance()
{
   mManager.RecordCost(mName, ofGetSystemTimeNanos() - mTimerStart);
}

void PerformanceTimer::RecordCost(std::string name, long cost)
{
   mCostTable.push_back(PerformanceTimer::Cost(name, cost));
}

bool PerformanceTimer::SortCosts(const PerformanceTimer::Cost& a, const PerformanceTimer::Cost& b)
{
   return a.mCost < b.mCost;
}

void PerformanceTimer::PrintCosts()
{
   sort(mCostTable.begin(), mCostTable.end(), SortCosts);
   for (Cost cost : mCostTable)
      ofLog() << cost.mName << " " << ofToString(cost.mCost);
}

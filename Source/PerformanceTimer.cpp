//
//  PerformanceTimer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/26/15.
//
//

#include "PerformanceTimer.h"
#include "SynthGlobals.h"

TimerInstance::TimerInstance(string name, PerformanceTimer& manager)
: mName(name)
, mManager(manager)
{
   mTimerStart = ofGetSystemTimeNanos();
}

TimerInstance::~TimerInstance()
{
   mManager.RecordCost(mName, ofGetSystemTimeNanos() - mTimerStart);
}

void PerformanceTimer::RecordCost(string name, long cost)
{
   mCostTable.push_back(PerformanceTimer::Cost(name,cost));
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

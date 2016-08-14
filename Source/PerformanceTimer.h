//
//  PerformanceTimer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/26/15.
//
//

#ifndef __Bespoke__PerformanceTimer__
#define __Bespoke__PerformanceTimer__

#include "OpenFrameworksPort.h"

class PerformanceTimer;

class TimerInstance
{
public:
   TimerInstance(string name, PerformanceTimer& manager);
   ~TimerInstance();
private:
   long mTimerStart;
   string mName;
   PerformanceTimer& mManager;
};

class PerformanceTimer
{
public:
   void RecordCost(string name, long cost);
   void PrintCosts();
private:
   struct Cost
   {
      Cost(string name, long cost) : mName(name), mCost(cost) {}
      string mName;
      long mCost;
   };
   
   static bool SortCosts(const PerformanceTimer::Cost& a, const PerformanceTimer::Cost& b);
   
   vector<Cost> mCostTable;
};

#endif /* defined(__Bespoke__PerformanceTimer__) */

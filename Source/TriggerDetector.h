//
//  TriggerDetector.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/22/14.
//
//

#ifndef __modularSynth__TriggerDetector__
#define __modularSynth__TriggerDetector__

#include <iostream>
#include "RollingBuffer.h"

class TriggerDetector
{
public:
   TriggerDetector();
   void Process(float sample);
   float GetValue();
   bool CheckTriggered();
   void SetThreshold(float thresh) { mThreshold = thresh; }
   void Draw(int x, int y);
   
   float mSharpness;
   
private:
   float mThreshold;
   bool mTriggered;
   bool mWaitingForFall;
   RollingBuffer mHistory;
   float mAvg;
};

#endif /* defined(__modularSynth__TriggerDetector__) */

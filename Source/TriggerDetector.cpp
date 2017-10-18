//
//  TriggerDetector.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/22/14.
//
//

#include "TriggerDetector.h"
#include "SynthGlobals.h"

TriggerDetector::TriggerDetector()
: mTriggered(false)
, mThreshold(.2f)
, mWaitingForFall(false)
, mHistory(gSampleRate)
, mSharpness(0)
, mAvg(0)
{
   
}

void TriggerDetector::Process(float sample)
{
   if (mSharpness > 0)
   {
      float filter = 1 - mSharpness * .001f;
      mAvg = filter * mAvg + (1-filter)*sample;
      sample -= mAvg; //highpass
   }
   
   if (sample - mHistory.GetSample(100, 0) > mThreshold && sample > .1f && mWaitingForFall == false)
   {
      mTriggered = true;
      mWaitingForFall = true;
   }
   if (sample < mHistory.GetSample(100, 0))
      mWaitingForFall = false;
   
   mHistory.Write(sample, 0);
}

float TriggerDetector::GetValue()
{
   return mHistory.GetSample(1, 0);
}

bool TriggerDetector::CheckTriggered()
{
   bool triggered = mTriggered;
   mTriggered = false;
   return triggered;
}

void TriggerDetector::Draw(int x, int y)
{
   ofPushStyle();
   ofSetLineWidth(1);
   ofSetColor(0,255,0);
   for (int i=1; i<mHistory.Size()-1; ++i)
   {
      ofRect(x-i/50,y,1,-mHistory.GetSample(i, 0)*300);
   }
   ofPopStyle();
}

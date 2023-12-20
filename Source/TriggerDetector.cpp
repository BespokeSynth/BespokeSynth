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
//  TriggerDetector.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/22/14.
//
//

#include "TriggerDetector.h"
#include "SynthGlobals.h"

TriggerDetector::TriggerDetector()
: mHistory(gSampleRate)
{
}

void TriggerDetector::Process(float sample)
{
   if (mSharpness > 0)
   {
      float filter = 1 - mSharpness * .001f;
      mAvg = filter * mAvg + (1 - filter) * sample;
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
   ofSetColor(0, 255, 0);
   for (int i = 1; i < mHistory.Size() - 1; ++i)
   {
      ofRect(x - i / 50, y, 1, -mHistory.GetSample(i, 0) * 300);
   }
   ofPopStyle();
}

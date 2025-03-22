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
//  TriggerDetector.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/22/14.
//
//

#pragma once

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

   float mSharpness{ 0 };

private:
   float mThreshold{ .2 };
   bool mTriggered{ false };
   bool mWaitingForFall{ false };
   RollingBuffer mHistory;
   float mAvg{ 0 };
};

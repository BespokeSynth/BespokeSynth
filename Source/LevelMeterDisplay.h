/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2024 Ryan Challinor (contact: awwbees@gmail.com)

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
//  LevelMeterDisplay.h
//  BespokeSynth
//
//  Created by Ryan Challinor on 12/15/24.
//
//

#pragma once

#include "ChannelBuffer.h"
#include "PeakTracker.h"

class LevelMeterDisplay
{
public:
   LevelMeterDisplay();
   virtual ~LevelMeterDisplay();

   void Process(int channel, float* buffer, int bufferSize);
   void Draw(float x, float y, float width, float height, int numChannels);
   void SetLimit(float limit);

private:
   struct LevelMeter
   {
      PeakTracker mPeakTracker;
      PeakTracker mPeakTrackerSlow;
   };

   std::array<LevelMeter, ChannelBuffer::kMaxNumChannels> mLevelMeters;
};

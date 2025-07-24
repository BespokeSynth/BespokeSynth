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
//  LevelMeterDisplay.cpp
//  BespokeSynth
//
//  Created by Ryan Challinor on 12/15/24.
//
//

#include "LevelMeterDisplay.h"
#include "SynthGlobals.h"

LevelMeterDisplay::LevelMeterDisplay()
{
   for (size_t i = 0; i < mLevelMeters.size(); ++i)
   {
      mLevelMeters[i].mPeakTrackerSlow.SetDecayTime(3);
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(1);
   }
}

LevelMeterDisplay::~LevelMeterDisplay()
{
}

void LevelMeterDisplay::Process(int channel, float* buffer, int bufferSize)
{
   mLevelMeters[channel].mPeakTracker.Process(buffer, bufferSize);
   mLevelMeters[channel].mPeakTrackerSlow.Process(buffer, bufferSize);
}

void LevelMeterDisplay::Draw(double x, double y, double width, double height, int numChannels)
{
   for (int i = 0; i < numChannels; ++i)
   {
      double limit = mLevelMeters[i].mPeakTrackerSlow.GetLimit();

      const int kNumSegments = 20;
      const int kPaddingBetween = 1;
      const int kPaddingVertical = 2;
      double barHeight = (height - (kPaddingVertical * (numChannels - 1))) / numChannels;
      double segmentWidth = width / kNumSegments;
      for (int j = 0; j < kNumSegments; ++j)
      {
         ofPushStyle();
         ofFill();
         double level = mLevelMeters[i].mPeakTracker.GetPeak() / (limit > 0 ? limit : 1);
         double slowLevel = mLevelMeters[i].mPeakTrackerSlow.GetPeak() / (limit > 0 ? limit : 1);
         ofColor color(0, 255, 0);
         if (j > kNumSegments - 3)
            color.set(255, 0, 0);
         else if (j > kNumSegments - 6)
            color.set(255, 255, 0);

         if (slowLevel > 0 && ofClamp(static_cast<int>(slowLevel * kNumSegments), 0, kNumSegments - 1) == j)
            ofSetColor(color);
         else if (level > 0 && level >= j / static_cast<double>(kNumSegments))
            ofSetColor(color * .9);
         else
            ofSetColor(color * .5);
         ofRect(x + segmentWidth * j, y + i * (barHeight + 2), segmentWidth - kPaddingBetween, barHeight, 0);
         ofPopStyle();
      }

      if (mLevelMeters[i].mPeakTrackerSlow.GetLastHitLimitTime() > gTime - 1000)
      {
         ofPushStyle();
         ofSetColor(ofColor::red);
         DrawTextBold("clipped", x + 10, y + i * (barHeight + 2) + 7, 10.0);
         ofPopStyle();
      }
   }
}

void LevelMeterDisplay::SetLimit(double limit)
{
   for (size_t i = 0; i < mLevelMeters.size(); ++i)
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(limit);
}

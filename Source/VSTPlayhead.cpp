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
//  VSTPlayhead.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/20/16.
//
//

#include "VSTPlayhead.h"
#include "Transport.h"

bool VSTPlayhead::getCurrentPosition(juce::AudioPlayHead::CurrentPositionInfo& result)
{
   result.bpm = TheTransport->GetTempo();
   result.timeSigNumerator = TheTransport->GetTimeSigTop();
   result.timeSigDenominator = TheTransport->GetTimeSigBottom();
   result.timeInSamples = gTime * gSampleRateMs;
   result.timeInSeconds = gTime / 1000.;

   /*
   * getMeasureTime is a float of how many measures we are through with fractional
   * measures. We want to know the number of quarter notes from the epoch which is
   * just the tsRatio times measure count, and for start of measure we simply floor
   * the measure time
   */
   double tsRatio = 4;
   if (result.timeSigDenominator > 0)
      tsRatio = 1.0 * result.timeSigNumerator / result.timeSigDenominator * 4;
   result.ppqPosition = (TheTransport->GetMeasureTime(gTime)) * tsRatio;
   result.ppqPositionOfLastBarStart = floor(TheTransport->GetMeasureTime(gTime)) * tsRatio;

   result.isPlaying = true;
   result.isRecording = false;
   result.isLooping = false;
   result.ppqLoopStart = 0;
   result.ppqLoopEnd = 480*result.timeSigDenominator;
   result.frameRate = AudioPlayHead::fps60;
   
   return true;
}

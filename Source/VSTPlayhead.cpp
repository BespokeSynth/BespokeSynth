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

juce::Optional<juce::AudioPlayHead::PositionInfo> VSTPlayhead::getPosition() const
{
   PositionInfo pos;
   juce::AudioPlayHead::TimeSignature timeSignature;
   juce::AudioPlayHead::LoopPoints loopPoint;
   timeSignature.numerator = TheTransport->GetTimeSigTop();
   timeSignature.denominator = TheTransport->GetTimeSigBottom();

   juce::Optional<juce::AudioPlayHead::TimeSignature> posTM = pos.getTimeSignature();

   loopPoint.ppqStart = 0;
   loopPoint.ppqEnd = 480;

   if (posTM)
   {
      loopPoint.ppqEnd *= posTM->denominator;
   }

   pos.setBpm(TheTransport->GetTempo());
   pos.setTimeSignature(timeSignature);
   pos.setTimeInSamples(int64_t(gTime * gSampleRateMs));
   pos.setTimeInSeconds(gTime / 1000);

   /*
   * getMeasureTime is a float of how many measures we are through with fractional
   * measures. We want to know the number of quarter notes from the epoch which is
   * just the tsRatio times measure count, and for start of measure we simply floor
   * the measure time
   */

   double tsRatio = 4;
   if (pos.getTimeSignature()->numerator > 0)
      tsRatio = 1.0 * pos.getTimeSignature()->numerator / pos.getTimeSignature()->denominator * 4;
   pos.setPpqPosition((TheTransport->GetMeasureTime(gTime)) * tsRatio);
   pos.setPpqPositionOfLastBarStart(floor(TheTransport->GetMeasureTime(gTime)) * tsRatio);

   pos.setIsPlaying(true);
   pos.setIsRecording(false);
   pos.setIsLooping(false);
   pos.setLoopPoints(loopPoint);
   pos.setFrameRate(juce::AudioPlayHead::fps60);

   return pos;
}

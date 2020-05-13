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
   result.ppqPosition = (TheTransport->GetMeasureTime(gTime)) * 480 * 4;
   result.ppqPositionOfLastBarStart = TheTransport->GetMeasure(gTime) * 480 * 4;
   result.isPlaying = true;
   result.isRecording = false;
   result.isLooping = false;
   
   return true;
}

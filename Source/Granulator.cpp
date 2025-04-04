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
//  Granulator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 9/12/13.
//
//

#include "Granulator.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "ChannelBuffer.h"
#include "juce_dsp/maths/juce_FastMathApproximations.h"

Granulator::Granulator()
{
   Reset();
}

void Granulator::Reset()
{
   mSpeed = 1;
   mGrainLengthMs = 60;
   mGrainOverlap = 10;
   mPosRandomizeMs = 5;
   mSpeedRandomize = 0;
   mSpacingRandomize = 1;
   mOctaves = false;
   mWidth = 1;

   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mBiquad[i].SetFilterParams(10, sqrt(2) / 2);
      mBiquad[i].SetFilterType(kFilterType_Highpass);
      mBiquad[i].UpdateFilterCoeff();
      mBiquad[i].Clear();
   }
}

void Granulator::ProcessFrame(double time, ChannelBuffer* buffer, int bufferLength, double offset, float speed, float* output)
{
   if (time + gInvSampleRateMs >= mNextGrainSpawnMs)
   {
      double startFromMs = mNextGrainSpawnMs;
      if (startFromMs < time - 1000) //must have recently started processing, reset
         startFromMs = time;
      SpawnGrain(mNextGrainSpawnMs, offset, buffer->NumActiveChannels() == 2 ? mWidth : 0, speed);
      mNextGrainSpawnMs = startFromMs + mGrainLengthMs * 1 / mGrainOverlap * ofRandom(1 - mSpacingRandomize / 2, 1 + mSpacingRandomize / 2);
   }

   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].Process(time, buffer, bufferLength, output);

   for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
   {
      if (mGrainOverlap > 4)
         output[ch] *= ofMap(mGrainOverlap, MAX_GRAINS, 4, .5f, 1); //lower volume on dense granulation, starting at 4 overlap
      output[ch] = mBiquad[ch].Filter(output[ch]);
   }
}

void Granulator::SpawnGrain(double time, double offset, float width, float speed)
{
   if (mLiveMode)
   {
      float speedMult = speed + mSpeedRandomize;
      if (mOctaves)
         speedMult *= 1.5f;
      float extraSpeed = MAX(0, speedMult * mSpeed - 1);
      float extraMs = mGrainLengthMs * extraSpeed + mPosRandomizeMs;
      float extraSamples = extraMs / gInvSampleRateMs;
      offset -= extraSamples;
   }
   float speedMult = speed + ofRandom(-mSpeedRandomize, mSpeedRandomize);
   float vol = 1;
   if (mOctaves)
   {
      int random = gRandom() % 5;
      if (random == 2) //fewer high-pitched ones
      {
         speedMult *= 1.5f;
         vol = .5f;
      }
      else if (random == 3 || random == 4)
      {
         speedMult *= .75f;
      }
   }
   offset += ofRandom(-mPosRandomizeMs, mPosRandomizeMs) / gInvSampleRateMs;
   mGrains[mNextGrainIdx].Spawn(this, time, offset, speedMult, mGrainLengthMs, vol, width);

   mNextGrainIdx = (mNextGrainIdx + 1) % MAX_GRAINS;
}

void Granulator::Draw(float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength)
{
   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].DrawGrain(i, x, y, w, h, bufferStart, viewLength, bufferLength);
}

void Granulator::ClearGrains()
{
   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].Clear();
}

void Grain::Spawn(Granulator* owner, double time, double pos, float speedMult, float lengthInMs, float vol, float width)
{
   mOwner = owner;
   mPos = pos;
   mSpeedMult = speedMult;
   mStartTime = time;
   mEndTime = time + lengthInMs;
   mStartToEnd = mEndTime - mStartTime;
   mStartToEndInv = 1.0 / mStartToEnd;
   mVol = vol;
   mStereoPosition = ofRandom(-width, width);
   mDrawPos = ofRandom(1);
}


inline double Grain::GetWindow(double time)
{
   double phase = (time - mStartTime) * mStartToEndInv;
   return .5 + .5 * juce::dsp::FastMathApproximations::cos<double>(phase * TWO_PI - PI);
}

void Grain::Process(double time, ChannelBuffer* buffer, int bufferLength, float* output)
{
   if (time >= mStartTime && time <= mEndTime && mVol != 0)
   {
      mPos += mSpeedMult * mOwner->mSpeed;
      float window = GetWindow(time);
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float sample = GetInterpolatedSample(mPos, buffer, bufferLength, std::clamp(ch + mStereoPosition, 0.f, 1.f));
         output[ch] += sample * window * mVol * (1 + (ch == 0 ? mStereoPosition : -mStereoPosition));
      }
   }
}

void Grain::DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength)
{
   float a = fmod((mPos - bufferStart), bufferLength) / viewLength;
   if (a < 0 || a > 1)
      return;
   ofPushStyle();
   ofFill();
   float alpha = GetWindow(std::clamp(gTime, mStartTime, mEndTime));
   ofSetColor(255, 0, 0, alpha * 255);
   ofCircle(x + a * w, y + mDrawPos * h, MAX(3, h / MAX_GRAINS / 2));
   ofPopStyle();
}

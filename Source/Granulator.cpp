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
   while (true)
   {
      double spawnTime;
      if (mPendingQueuedGrainSpawnTime != -1) // don't consume any more of the queue until we play this one
      {
         spawnTime = mPendingQueuedGrainSpawnTime;
         mPendingQueuedGrainSpawnTime = -1;
      }
      else
      {
         bool hasData = mQueuedGrainSpawnTimes.try_dequeue(spawnTime);
         if (!hasData)
            break;
      }

      bool spawned = SpawnGrainIfReady(time, spawnTime, buffer, offset, speed);
      if (!spawned) // haven't gotten to spawnTime yet, hold onto this spawn time
      {
         mPendingQueuedGrainSpawnTime = spawnTime;
         break;
      }
   }

   if (mSpawnGrains)
      SpawnGrainIfReady(time, mNextGrainSpawnMs, buffer, offset, speed);

   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].Process(time, buffer, bufferLength, output, this);

   for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
   {
      if (mGrainOverlap > 4)
         output[ch] *= ofMap(mGrainOverlap, MAX_GRAINS, 4, .5f, 1); //lower volume on dense granulation, starting at 4 overlap
      output[ch] = mBiquad[ch].Filter(output[ch]);
   }
}

bool Granulator::SpawnGrainIfReady(double currentTime, double spawnTime, ChannelBuffer* buffer, double offset, float speed)
{
   if (currentTime + gInvSampleRateMs >= spawnTime)
   {
      double startFromMs = std::max(spawnTime, currentTime);
      SpawnGrain(startFromMs, offset, buffer->NumActiveChannels() == 2 ? mWidth : 0, speed);
      return true;
   }
   return false;
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
   mGrains[mNextGrainIdx].Spawn(time, offset, speedMult, mGrainLengthMs, vol, width);

   mNextGrainIdx = (mNextGrainIdx + 1) % MAX_GRAINS;
   mNextGrainSpawnMs = time + mGrainLengthMs * 1 / mGrainOverlap * ofRandom(1 - mSpacingRandomize / 2, 1 + mSpacingRandomize / 2);
}

void Granulator::QueueGrainSpawn(double spawnTime)
{
   mQueuedGrainSpawnTimes.enqueue(spawnTime);
}

void Granulator::Draw(float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength, float gain)
{
   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].DrawGrain(i, x, y, w, h, bufferStart, viewLength, bufferLength, gain, this);
}

void Granulator::DrawWindow(float x, float y, float w, float h)
{
   ofPushStyle();

   ofSetColor(100, 100, 100, .8f * gModuleDrawAlpha);
   ofSetLineWidth(.5f);
   ofFill();
   ofRect(x, y, w, h, 0);

   ofSetColor(245, 58, 0, gModuleDrawAlpha);
   ofSetLineWidth(1);
   ofNoFill();
   ofBeginShape();
   const int stepSize = 3;
   for (int i = 0; i < (int)w; i += stepSize)
   {
      ofVertex(x + i, y + h - GetWindow(mWindowType, mWindowShape, mGrainLengthMs, i / w) * h);
   }
   ofEndShape();

   ofPopStyle();
}

void Granulator::ClearGrains()
{
   for (int i = 0; i < MAX_GRAINS; ++i)
      mGrains[i].Clear();
}

namespace
{
   inline static double FastCosWindow(double x)
   {
      double factor = 5.385;
      double z = 2.0 * factor * x - factor;
      double z2 = z * z;
      double z4 = z2 * z2;

      // [4/4] Pade approximant of cos(z)
      double cos_approx = (z4 - 56.0 * z2 + 840.0) /
                          (z4 + 28.0 * z2 + 840.0);

      return cos_approx;
   }
}

inline double Granulator::GetWindow(GrainWindowType type, double shape, double grainLengthMs, double phase)
{
   if (type == GrainWindowType::Round)
   {
      if (phase < shape)
         phase = phase / shape * 0.5;
      else
         phase = (phase - shape) / (1.0 - shape) * 0.5 + 0.5;
      return .5 + .5 * juce::dsp::FastMathApproximations::cos<double>(phase * TWO_PI - PI);
   }
   else if (type == GrainWindowType::Fast)
   {
      if (phase < shape)
         phase = phase / shape * 0.5;
      else
         phase = (phase - shape) / (1.0 - shape) * 0.5 + 0.5;
      return FastCosWindow(phase);
   }
   else if (type == GrainWindowType::Triangle)
   {
      if (phase < shape)
         return phase / shape;
      return 1.0 - ((phase - shape) / (1.0 - shape));
   }
   else if (type == GrainWindowType::Envelope)
   {
      const double kFadeInMs = 1.0;
      const double kFadeIn = kFadeInMs / grainLengthMs;
      if (phase < kFadeIn)
         return phase / kFadeIn;
      if (phase > shape)
         return 1.0 - ((phase - shape) / (1.0 - shape));
      if (phase >= 1.0)
         return 0.0;
      return 1.0;
   }
   else if (type == GrainWindowType::Hybrid)
   {
      return (GetWindow(GrainWindowType::Round, 0.5, grainLengthMs, phase) + GetWindow(GrainWindowType::Envelope, shape, grainLengthMs, phase)) * 0.5;
   }
   return 0.0;
}

void Grain::Spawn(double time, double pos, float speedMult, float lengthInMs, float vol, float width)
{
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

void Grain::Process(double time, ChannelBuffer* buffer, int bufferLength, float* output, const Granulator* granulator)
{
   if (time >= mStartTime && time <= mEndTime && mVol != 0)
   {
      mPos += mSpeedMult * granulator->mSpeed;
      double phase = (time - mStartTime) * mStartToEndInv;
      double window = Granulator::GetWindow(granulator->mWindowType, granulator->mWindowShape, granulator->mGrainLengthMs, phase);
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float sample = GetInterpolatedSample(mPos, buffer, bufferLength, std::clamp(ch + mStereoPosition, 0.f, 1.f));
         output[ch] += sample * window * mVol * (1 + (ch == 0 ? mStereoPosition : -mStereoPosition));
      }
   }
}

void Grain::DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int viewLength, int bufferLength, float gain, const Granulator* granulator)
{
   float a = fmod((mPos - bufferStart), bufferLength) / viewLength;
   if (a < 0 || a > 1)
      return;
   ofPushStyle();
   ofFill();
   double phase = (std::clamp(gTime, mStartTime, mEndTime) - mStartTime) * mStartToEndInv;
   float alpha = Granulator::GetWindow(granulator->mWindowType, granulator->mWindowShape, granulator->mGrainLengthMs, phase) * gain;
   ofSetColor(255, 0, 0, alpha * 255);
   ofCircle(x + a * w, y + mDrawPos * h, MAX(3, h / MAX_GRAINS / 2));
   ofPopStyle();
}

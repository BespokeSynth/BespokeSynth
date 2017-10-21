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

Granulator::Granulator()
: mNextGrainIdx(0)
, mLastGrainSpawnMs(0)
, mLiveMode(false)
, mOctaves(false)
{
   Reset();
}

void Granulator::Reset()
{
   mSpeed = 1;
   mGrainLengthMs = 60;
   mGrainSpacing = .1f;
   mPosRandomizeMs = 5;
   mSpeedRandomize = 0;
   mSpacingRandomize = 0;
   mOctaves = false;
}

void Granulator::Process(double time, ChannelBuffer* buffer, int bufferLength, float offset, float* output)
{
   if (time >= mLastGrainSpawnMs+mGrainLengthMs*mGrainSpacing*ofRandom(1-mSpacingRandomize/2,1+mSpacingRandomize/2))
   {
      mLastGrainSpawnMs = time;
      SpawnGrain(time, offset);
   }
   
   for (int i=0; i<MAX_GRAINS; ++i)
      mGrains[i].Process(time, buffer, bufferLength, output);
   
   if (mGrainSpacing < .25f)
   {
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         output[ch] *= ofMap(mGrainSpacing,0,.25f,.15f,1);   //lower volume on dense granulation, starting at .25 spacing
   }
}

void Granulator::SpawnGrain(double time, float offset)
{
   if (mLiveMode)
   {
      float speed = mSpeed * (1+mSpeedRandomize);
      if (mOctaves)
         speed *= 1.5f;
      float extraSpeed = MAX(0,speed - 1);
      float extraMs = mGrainLengthMs * extraSpeed + mPosRandomizeMs;
      float extraSamples = extraMs / gInvSampleRateMs;
      offset -= extraSamples;
   }
   float speed = mSpeed * (1+ofRandom(-mSpeedRandomize, mSpeedRandomize));
   float vol = 1;
   if (mOctaves)
   {
      int random = rand() % 5;
      if (random == 2) //fewer high-pitched ones
      {
         speed *= 1.5f;
         vol = .5f;
      }
      else if (random == 3 || random == 4)
      {
         speed *= .75f;
      }
   }
   offset += ofRandom(-mPosRandomizeMs, mPosRandomizeMs) / gInvSampleRateMs;
   mGrains[mNextGrainIdx].Spawn(time, offset, speed, mGrainLengthMs, vol);
   
   mNextGrainIdx = (mNextGrainIdx+1) % MAX_GRAINS;
}

void Granulator::Draw(float x, float y, float w, float h, int bufferStart, int bufferLength, bool wrapAround /*= true*/)
{
   for (int i=0; i<MAX_GRAINS; ++i)
      mGrains[i].DrawGrain(i, x, y, w, h, bufferStart, bufferLength, wrapAround);
}

void Granulator::ClearGrains()
{
   for (int i=0; i<MAX_GRAINS; ++i)
      mGrains[i].Clear();
}

void Grain::Spawn(double time, float pos, float speed, float lengthInMs, float vol)
{
   mPos = pos;
   mSpeed = speed;
   mStartTime = time;
   mEndTime = time + lengthInMs;
   mVol = vol;
   mStereoPosition = ofRandom(1);
}

void Grain::Process(double time, ChannelBuffer* buffer, int bufferLength, float* output)
{
   float sample = GetInterpolatedSample(mPos, buffer, bufferLength, mStereoPosition);
   mPos += mSpeed;
   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
      output[ch] += sample * GetWindow(time) * mVol * (ch == 0 ? (1-mStereoPosition) : mStereoPosition);
}

float Grain::GetWindow(double time)
{
   if (time > mStartTime && time < mEndTime)
   {
      double phase = (time-mStartTime)/(mEndTime-mStartTime);
      return .5f * (1 - cos(phase * TWO_PI));
   }
   return 0;
}

void Grain::DrawGrain(int idx, float x, float y, float w, float h, int bufferStart, int bufferLength, bool wrapAround)
{
   float a = (mPos - bufferStart) / bufferLength;
   if (wrapAround)
      FloatWrap(a,1);
   if (a < 0 || a > 1)
      return;
   ofPushStyle();
   ofFill();
   float alpha = GetWindow(gTime);
   ofSetColor(255,0,0,alpha*alpha*255*.5);
   ofRect(x+a*w, y+(float(idx)/MAX_GRAINS)*h, MAX(1,w/100), h/MAX_GRAINS);
   ofPopStyle();
}

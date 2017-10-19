//
//  JumpBlender.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/1/13.
//
//

#include "JumpBlender.h"
#include "SynthGlobals.h"
#include "Profiler.h"

JumpBlender::JumpBlender()
: mBlending(false)
{
   
}

void JumpBlender::CaptureForJump(int pos, const float* sampleSource, int sourceLength, int samplesIn)
{
   assert(pos < sourceLength);
   
   mBlending = true;
   mBlendSample = 0;
   if (pos + JUMP_BLEND_SAMPLES <= sourceLength)
   {
      BufferCopy((float*)mSamples, sampleSource+pos, JUMP_BLEND_SAMPLES);
   }
   else
   {
      int samplesLeft = sourceLength - pos;
      BufferCopy(mSamples, sampleSource+pos, samplesLeft);
      BufferCopy(mSamples+samplesLeft, sampleSource, (JUMP_BLEND_SAMPLES-samplesLeft));
   }
   double time = gTime+samplesIn*gInvSampleRateMs;
   mRamp.Start(time, 1, 0, time+JUMP_BLEND_SAMPLES*gInvSampleRateMs);
}

float JumpBlender::Process(float sample, int samplesIn)
{
   if (mBlendSample == JUMP_BLEND_SAMPLES)
      mBlending = false;
   
   if (!mBlending)
      return sample;
   
   float rampVal = mRamp.Value(gTime+samplesIn*gInvSampleRateMs);
   return sample*(1-rampVal) + mSamples[mBlendSample++]*rampVal;
}

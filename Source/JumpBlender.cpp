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
{
}

void JumpBlender::CaptureForJump(int pos, const float* sampleSource, int sourceLength, int samplesIn)
{
   assert(pos < sourceLength);

   mBlending = true;
   mBlendSample = 0;
   if (pos + JUMP_BLEND_SAMPLES <= sourceLength)
   {
      BufferCopy((float*)mSamples, sampleSource + pos, JUMP_BLEND_SAMPLES);
   }
   else
   {
      int samplesLeft = sourceLength - pos;
      BufferCopy(mSamples, sampleSource + pos, samplesLeft);
      BufferCopy(mSamples + samplesLeft, sampleSource, (JUMP_BLEND_SAMPLES - samplesLeft));
   }
   double time = gTime + samplesIn * gInvSampleRateMs;
   mRamp.Start(time, 1, 0, time + JUMP_BLEND_SAMPLES * gInvSampleRateMs);
}

float JumpBlender::Process(float sample, int samplesIn)
{
   if (mBlendSample == JUMP_BLEND_SAMPLES)
      mBlending = false;

   if (!mBlending)
      return sample;

   float rampVal = mRamp.Value(gTime + samplesIn * gInvSampleRateMs);
   return sample * (1 - rampVal) + mSamples[mBlendSample++] * rampVal;
}

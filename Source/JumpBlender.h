//
//  JumpBlender.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/1/13.
//
//

#ifndef __modularSynth__JumpBlender__
#define __modularSynth__JumpBlender__

#include <iostream>
#include "Ramp.h"

#define JUMP_BLEND_SAMPLES 100

class JumpBlender
{
public:
   JumpBlender();
   
   void CaptureForJump(int pos, const float* sampleSource, int sourceLength, int samplesIn);
   float Process(float sample, int samplesIn);
private:
   bool mBlending;
   Ramp mRamp;
   float mSamples[JUMP_BLEND_SAMPLES];
   int mBlendSample;
};

#endif /* defined(__modularSynth__JumpBlender__) */

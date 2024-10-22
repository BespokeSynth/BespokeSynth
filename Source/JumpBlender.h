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
//  JumpBlender.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/1/13.
//
//

#pragma once

#include "Ramp.h"

#define JUMP_BLEND_SAMPLES 100

class JumpBlender
{
public:
   JumpBlender();

   void CaptureForJump(int pos, const float* sampleSource, int sourceLength, int samplesIn);
   float Process(float sample, int samplesIn);

private:
   bool mBlending{ false };
   Ramp mRamp;
   float mSamples[JUMP_BLEND_SAMPLES]{};
   int mBlendSample{ 0 };
};

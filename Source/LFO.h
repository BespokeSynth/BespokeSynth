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
//  LFO.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#pragma once

#include "SynthGlobals.h"
#include "Transport.h"
#include "Oscillator.h"
#include "Ramp.h"
#include "PerlinNoise.h"

enum LFOMode
{
   kLFOMode_Envelope,
   kLFOMode_Oscillator
};

class LFO : public ITimeListener, public IAudioPoller
{
public:
   LFO();
   ~LFO();
   float Value(int samplesIn = 0, float forcePhase = -1) const;
   void SetOffset(float offset)
   {
      mPhaseOffset = offset;
      mAdjustOffset = 0;
   }
   void SetPeriod(NoteInterval interval);
   void SetType(OscillatorType type);
   void SetPulseWidth(float width) { mOsc.SetPulseWidth(width); }
   void SetMode(LFOMode mode) { mMode = mode; }
   float CalculatePhase(int samplesIn = 0, bool doTransform = true) const;
   Oscillator* GetOsc() { return &mOsc; }
   void SetFreeRate(float rate) { mFreeRate = rate; }
   void SetLength(float length) { mLength = length; }
   float TransformPhase(float phase) const;
   void ResetPhase(double time);

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

private:
   NoteInterval mPeriod{ NoteInterval::kInterval_1n };
   float mPhaseOffset{ 0 };
   Oscillator mOsc{ OscillatorType::kOsc_Sin };
   LFOMode mMode{ LFOMode::kLFOMode_Envelope };
   Ramp mRandom;
   float mDrunk{ 0 };
   double mFreePhase{ 0 };
   float mFreeRate{ 1 };
   float mLength{ 1 };
   int mPerlinSeed{ 0 };
   float mAdjustOffset{ 0 };

   static PerlinNoise sPerlinNoise;
};

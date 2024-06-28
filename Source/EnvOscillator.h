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
//  EnvOscillator.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#pragma once

#include "SynthGlobals.h"
#include "ADSR.h"
#include "Oscillator.h"

class EnvOscillator
{
public:
   EnvOscillator(OscillatorType type)
   : mOsc(type)
   {}

   void SetType(OscillatorType type) { mOsc.SetType(type); }
   void SetADSR(float a, float d, float s, float r) { mAdsr.Set(a, d, s, r); }
   void Start(double time, float target) { mAdsr.Start(time, target); }
   void Start(double time, float target, float a, float d, float s, float r) { mAdsr.Start(time, target, a, d, s, r); }
   void Start(double time, float target, ::ADSR adsr)
   {
      mAdsr.Set(adsr);
      mAdsr.Start(time, target);
   }
   void Stop(double time) { mAdsr.Stop(time); }
   float Audio(double time, float phase);
   ::ADSR* GetADSR() { return &mAdsr; }
   void SetPulseWidth(float width) { mOsc.SetPulseWidth(width); }
   Oscillator mOsc{ OscillatorType::kOsc_Sin };

private:
   ::ADSR mAdsr;
   float mPulseWidth{ .5 };
};

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
//  Oscillator.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/3/14.
//
//

#pragma once

#include "SynthGlobals.h"
#include "ADSR.h"

class Oscillator
{
public:
   Oscillator(OscillatorType type)
   : mType(type)
   {}

   enum class SyncMode
   {
      None,
      Ratio,
      Frequency
   };

   OscillatorType GetType() const { return mType; }
   void SetType(OscillatorType type) { mType = type; }
   float Value(float phase) const;
   float GetPulseWidth() const { return mPulseWidth; }
   void SetPulseWidth(float width) { mPulseWidth = width; }
   float GetShuffle() const { return mShuffle; }
   void SetShuffle(float shuffle) { mShuffle = MIN(shuffle, .999f); }
   float GetSoften() const { return mSoften; }
   void SetSoften(float soften) { mSoften = ofClamp(soften, 0, 1); }
   OscillatorType mType{ OscillatorType::kOsc_Sin };

private:
   float SawSample(float phase) const;

   float mPulseWidth{ .5 };
   float mShuffle{ 0 };
   float mSoften{ 0 };
};

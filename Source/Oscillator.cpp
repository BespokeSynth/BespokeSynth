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
//  Oscillator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 7/3/14.
//
//

#include "Oscillator.h"

float Oscillator::Value(float phase) const
{
   if (mType == kOsc_Tri)
      phase += .5f * FPI; //shift phase to make triangle start at zero instead of 1, to eliminate click on start

   if (mShuffle > 0)
   {
      phase = fmod(phase, FTWO_PI * 2);

      float shufflePoint = FTWO_PI * (1 + mShuffle);

      if (phase < shufflePoint)
         phase = phase / (1 + mShuffle);
      else
         phase = (phase - shufflePoint) / (1 - mShuffle);
   }

   phase = fmod(phase, FTWO_PI);

   float sample = 0;
   switch (mType)
   {
      case kOsc_Sin:
         sample = sin(phase);
         break;
      case kOsc_Saw:
         sample = SawSample(phase);
         break;
      case kOsc_NegSaw:
         sample = -SawSample(phase);
         break;
      case kOsc_Square:
         if (mSoften == 0)
         {
            sample = phase > (FTWO_PI * mPulseWidth) ? -1 : 1;
         }
         else
         {
            float phase01 = phase / FTWO_PI;
            phase01 += .75f;
            phase01 -= (mPulseWidth - .5f) / 2;
            phase01 -= int(phase01);
            sample = ofClamp((fabs(phase01 - .5f) * 4 - 1 + (mPulseWidth - .5f) * 2) / mSoften, -1, 1);
         }
         break;
      case kOsc_Tri:
         sample = fabs(phase / FTWO_PI - .5f) * 4 - 1;
         break;
      case kOsc_Random:
         sample = ofRandom(-1, 1);
         break;
      default:
         //assert(false);
         break;
   }

   if (mType != kOsc_Square && mPulseWidth != .5f)
      sample = (Bias(sample / 2 + .5f, mPulseWidth) - .5f) * 2; //give "pulse width" to non-square oscillators

   return sample;
}

float Oscillator::SawSample(float phase) const
{
   phase /= FTWO_PI;
   if (mSoften == 0)
      return phase * 2 - 1;
   if (phase < 1 - mSoften)
      return phase / (1 - mSoften) * 2 - 1;
   return 1 - ((phase - (1 - mSoften)) / mSoften * 2);
}

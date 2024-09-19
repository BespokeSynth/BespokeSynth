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

double Oscillator::Value(double phase) const
{
   if (mType == kOsc_Tri)
      phase += .5 * PI; //shift phase to make triangle start at zero instead of 1, to eliminate click on start

   if (mShuffle > 0)
   {
      phase = fmod(phase, TWO_PI * 2);

      double shufflePoint = TWO_PI * (1 + mShuffle);

      if (phase < shufflePoint)
         phase = phase / (1 + mShuffle);
      else
         phase = (phase - shufflePoint) / (1 - mShuffle);
   }

   phase = fmod(phase, TWO_PI);

   double sample = 0;
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
         if (ofAlmostEquel(mSoften, 0.0))
         {
            sample = phase > (TWO_PI * mPulseWidth) ? -1 : 1;
         }
         else
         {
            double phase01 = phase / TWO_PI;
            phase01 += .75;
            phase01 -= (mPulseWidth - .5) / 2;
            phase01 -= static_cast<int>(phase01);
            sample = ofClamp((abs(phase01 - .5) * 4 - 1 + (mPulseWidth - .5) * 2) / mSoften, -1, 1);
         }
         break;
      case kOsc_Tri:
         sample = abs(phase / TWO_PI - .5) * 4 - 1;
         break;
      case kOsc_Random:
         sample = ofRandom(-1.0, 1.0);
         break;
      default:
         //assert(false);
         break;
   }

   if (mType != kOsc_Square && mPulseWidth != .5)
      sample = (Bias(sample / 2 + .5, mPulseWidth) - .5) * 2; //give "pulse width" to non-square oscillators

   return sample;
}

double Oscillator::SawSample(double phase) const
{
   phase /= TWO_PI;
   if (ofAlmostEquel(mSoften, 0.0))
      return phase * 2 - 1;
   if (phase < 1 - mSoften)
      return phase / (1 - mSoften) * 2 - 1;
   return 1 - ((phase - (1 - mSoften)) / mSoften * 2);
}

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
   if (mShuffle > 0)
   {
      while (phase > FTWO_PI * 2) { phase -= FTWO_PI * 2; }
      
      float shufflePoint = FTWO_PI * (1+mShuffle);
      
      if (phase < shufflePoint)
         phase = phase / (1+mShuffle);
      else
         phase = (phase - shufflePoint) / (1-mShuffle);
   }
   
   while (phase > FTWO_PI) { phase -= FTWO_PI; }
   
   float sample;
   switch (mType)
   {
      case kOsc_Sin:
         sample = sin(phase);
         break;
      case kOsc_Saw:
         sample = phase/FTWO_PI * 2 - 1;
         break;
      case kOsc_NegSaw:
         sample = 1 - phase/FTWO_PI * 2;
         break;
      case kOsc_Square:
         sample = phase > (FTWO_PI * mPulseWidth) ? -1 : 1;
         break;
      case kOsc_Tri:
         sample = fabs(phase/FTWO_PI - .5f) * 4 - 1;
         break;
      case kOsc_Random:
         sample = ofRandom(-1,1);
         break;
      case kOsc_Drunk:
         assert(false);
         break;
   }
   
   if (mType != kOsc_Square && mPulseWidth != .5f)
      sample = (Bias(sample/2+.5f, mPulseWidth) - .5f) * 2; //give "pulse width" to non-square oscillators
   
   return sample;
}


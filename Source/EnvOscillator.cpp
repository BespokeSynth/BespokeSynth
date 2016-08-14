//
//  EnvOscillator.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#include "EnvOscillator.h"

float EnvOscillator::Audio(double time, float phase)
{
   float sample = mOsc.Value(phase) * mAdsr.Value(time);
   
   if (mOsc.GetType() == kOsc_Square ||
       mOsc.GetType() == kOsc_Saw ||
       mOsc.GetType() == kOsc_NegSaw)
      sample *= .5f; //cut down apparent loudness compared to sin and tri
   
   return sample;
}

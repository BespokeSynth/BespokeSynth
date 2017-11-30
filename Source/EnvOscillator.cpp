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
   return mOsc.Value(phase) * mAdsr.Value(time);
}

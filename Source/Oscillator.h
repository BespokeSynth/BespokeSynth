//
//  Oscillator.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/3/14.
//
//

#ifndef __Bespoke__Oscillator__
#define __Bespoke__Oscillator__

#include <iostream>
#include "SynthGlobals.h"
#include "ADSR.h"

class Oscillator
{
public:
   Oscillator(OscillatorType type) : mType(type), mPulseWidth(.5f), mShuffle(0) {}
   
   OscillatorType GetType() const { return mType; }
   void SetType(OscillatorType type) { mType = type; }
   float Value(float phase) const;
   float GetPulseWidth() const { return mPulseWidth; }
   void SetPulseWidth(float width) { mPulseWidth = width; }
   void SetShuffle(float shuffle) { mShuffle = MIN(shuffle, .999f); }
   OscillatorType mType;
private:
   float mPulseWidth;
   float mShuffle;
};

#endif /* defined(__Bespoke__Oscillator__) */

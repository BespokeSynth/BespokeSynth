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
   Oscillator(OscillatorType type) : mType(type), mPulseWidth(.5f), mShuffle(0), mSoften(0) {}
   
   OscillatorType GetType() const { return mType; }
   void SetType(OscillatorType type) { mType = type; }
   float Value(float phase) const;
   float GetPulseWidth() const { return mPulseWidth; }
   void SetPulseWidth(float width) { mPulseWidth = width; }
   float GetShuffle() const { return mShuffle; }
   void SetShuffle(float shuffle) { mShuffle = MIN(shuffle, .999f); }
   float GetSoften() const { return mSoften; }
   void SetSoften(float soften) { mSoften = ofClamp(soften,0,1); }
   OscillatorType mType;
private:
   float SawSample(float phase) const;
   
   float mPulseWidth;
   float mShuffle;
   float mSoften;
};

#endif /* defined(__Bespoke__Oscillator__) */

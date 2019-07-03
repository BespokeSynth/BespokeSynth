//
//  EnvOscillator.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#ifndef __additiveSynth__EnvOscillator__
#define __additiveSynth__EnvOscillator__

#include <iostream>
#include "SynthGlobals.h"
#include "ADSR.h"
#include "Oscillator.h"

class EnvOscillator
{
public:
   EnvOscillator(OscillatorType type) : mOsc(type), mPulseWidth(.5f) {}
   
   void SetType(OscillatorType type) { mOsc.SetType(type); }
   void SetADSR(float a, float d, float s, float r) { mAdsr.Set(a,d,s,r); }
   void Start(double time, float target) { mAdsr.Start(time, target); }
   void Start(double time, float target, float a, float d, float s, float r) { mAdsr.Start(time, target, a, d, s, r); }
   void Start(double time, float target, ::ADSR adsr) { mAdsr.Set(adsr); mAdsr.Start(time, target); }
   void Stop(double time) { mAdsr.Stop(time); }
   float Audio(double time, float phase);
   ::ADSR* GetADSR() { return &mAdsr; }
   void SetPulseWidth(float width) { mOsc.SetPulseWidth(width); }
   Oscillator mOsc;
private:
   ::ADSR mAdsr;
   float mPulseWidth;
};

#endif /* defined(__additiveSynth__EnvOscillator__) */

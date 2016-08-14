//
//  DCRemoverEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/2/14.
//
//

#ifndef __Bespoke__DCRemoverEffect__
#define __Bespoke__DCRemoverEffect__

#include <stdio.h>
#include <iostream>
#include "IAudioProcessor.h"
#include "BiquadFilter.h"

class DCRemoverEffect : public IAudioProcessor
{
public:
   DCRemoverEffect();
   ~DCRemoverEffect();
   
   static IAudioProcessor* Create() { return new DCRemoverEffect(); }
   
   string GetTitleLabel() override { return "dc remover"; }
   
   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "dcremover"; }
   
private:
   //IDrawableModule
   void GetModuleDimensions(int& width, int& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   BiquadFilter mBiquad;
};

#endif /* defined(__Bespoke__DCRemoverEffect__) */

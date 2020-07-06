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
#include "IAudioEffect.h"
#include "BiquadFilter.h"

class DCRemoverEffect : public IAudioEffect
{
public:
   DCRemoverEffect();
   ~DCRemoverEffect();
   
   static IAudioEffect* Create() { return new DCRemoverEffect(); }
   
   string GetTitleLabel() override { return "dc remover"; }
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "dcremover"; }
   
private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels];
};

#endif /* defined(__Bespoke__DCRemoverEffect__) */

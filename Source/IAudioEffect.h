//
//  IAudioEffect.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#ifndef additiveSynth_IAudioEffect_h
#define additiveSynth_IAudioEffect_h

#include "IDrawableModule.h"
#include "ChannelBuffer.h"

class IAudioEffect : public IDrawableModule
{
public:
   virtual ~IAudioEffect() {}
   virtual void ProcessAudio(double time, ChannelBuffer* buffer) = 0;
   void SetEnabled(bool enabled) override = 0;
   virtual float GetEffectAmount() { return 0; }
   virtual string GetType() = 0;
   bool CanMinimize() override { return false; }
   bool IsSaveable() override { return false; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override {}
};

#endif

//
//  IAudioProcessor.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#ifndef additiveSynth_IAudioProcessor_h
#define additiveSynth_IAudioProcessor_h

#include "IDrawableModule.h"

class IAudioProcessor : public IDrawableModule
{
public:
   virtual ~IAudioProcessor() {}
   virtual void ProcessAudio(double time, float* audio, int bufferSize) = 0;
   void SetEnabled(bool enabled) override = 0;
   virtual float GetEffectAmount() { return 0; }
   virtual string GetType() = 0;
   bool CanMinimize() override { return false; }
   bool IsSaveable() override { return false; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override {}
};

#endif

//
//  InputChannel.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/12.
//
//

#ifndef __modularSynth__InputChannel__
#define __modularSynth__InputChannel__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"

class InputChannel : public IAudioProcessor, public IDrawableModule
{
public:
   InputChannel();
   virtual ~InputChannel();
   static IDrawableModule* Create() { return new InputChannel(); }
   
   string GetTitleLabel() override { return "in "+ofToString(mChannel); }
   
   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=40; height=0; }
   bool Enabled() const override { return mEnabled; }
   
   int mChannel;
};

#endif /* defined(__modularSynth__InputChannel__) */

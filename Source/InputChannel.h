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
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"

class InputChannel : public IAudioReceiver, public IAudioSource, public IDrawableModule
{
public:
   InputChannel();
   virtual ~InputChannel();
   static IDrawableModule* Create() { return new InputChannel(); }
   
   string GetTitleLabel() override { return "in "+ofToString(mChannel); }
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int&y) override { x=40; y=0; }
   bool Enabled() const override { return mEnabled; }
   
   int mChannel;
   
   int mInputBufferSize;
   float* mInputBuffer;
};

#endif /* defined(__modularSynth__InputChannel__) */

//
//  OutputChannel.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/17/12.
//
//

#ifndef __modularSynth__OutputChannel__
#define __modularSynth__OutputChannel__

#include <iostream>
#include "IAudioReceiver.h"
#include "IDrawableModule.h"

class OutputChannel : public IAudioReceiver, public IDrawableModule
{
public:
   OutputChannel();
   virtual ~OutputChannel();
   static IDrawableModule* Create() { return new OutputChannel(); }
   
   string GetTitleLabel() override { return "out "+ofToString(mChannel); }
   
   void ClearBuffer();
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int&y) override { x=50; y=0; }
   bool Enabled() const override { return true; }
   
   int mChannel;
   int mInputBufferSize;
   float* mInputBuffer;
};

#endif /* defined(__modularSynth__OutputChannel__) */

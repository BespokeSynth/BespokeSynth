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
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "DropdownList.h"

class OutputChannel : public IAudioProcessor, public IDrawableModule, public IDropdownListener
{
public:
   OutputChannel();
   virtual ~OutputChannel();
   static IDrawableModule* Create() { return new OutputChannel(); }
   
   string GetTitleLabel() override { return "output"; }
   void CreateUIControls() override;
   
   //IAudioReceiver
   InputMode GetInputMode() override { return mChannelSelectionIndex < mStereoSelectionOffset ? kInputMode_Mono : kInputMode_Multichannel; }
   
   //IAudioSource
   void Process(double time) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=64; height=20; }
   bool Enabled() const override { return true; }
   
   DropdownList* mChannelSelector;
   int mChannelSelectionIndex;
   int mStereoSelectionOffset;
};

#endif /* defined(__modularSynth__OutputChannel__) */

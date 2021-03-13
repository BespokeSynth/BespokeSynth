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
#include "DropdownList.h"

class InputChannel : public IAudioProcessor, public IDrawableModule, public IDropdownListener
{
public:
   InputChannel();
   virtual ~InputChannel();
   static IDrawableModule* Create() { return new InputChannel(); }
   
   string GetTitleLabel() override { return "input"; }
   void CreateUIControls();
   
   //IAudioReceiver
   InputMode GetInputMode() override { return mChannelSelectionIndex < mStereoSelectionOffset ? kInputMode_Mono : kInputMode_Multichannel; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 64; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   DropdownList* mChannelSelector;
   int mChannelSelectionIndex;
   int mStereoSelectionOffset;
};

#endif /* defined(__modularSynth__InputChannel__) */

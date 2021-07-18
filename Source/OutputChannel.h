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
#include "PeakTracker.h"

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
   void GetModuleDimensions(float& width, float& height) override { width=mWidth; height=mHeight; }
   bool Enabled() const override { return true; }
   
   int GetNumChannels() const { return mChannelSelectionIndex < mStereoSelectionOffset ? 1 : 2; }
   
   float mWidth;
   float mHeight;
   DropdownList* mChannelSelector;
   int mChannelSelectionIndex;
   int mStereoSelectionOffset;
   
   struct LevelMeter
   {
      float mLevel;
      float mMaxLevel;
      PeakTracker mPeakTracker;
      PeakTracker mPeakTrackerSlow;
   };
   
   std::array<LevelMeter,2> mLevelMeters;
};

#endif /* defined(__modularSynth__OutputChannel__) */

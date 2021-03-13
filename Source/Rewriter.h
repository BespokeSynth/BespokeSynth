//
//  Rewriter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#ifndef __modularSynth__Rewriter__
#define __modularSynth__Rewriter__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "MidiDevice.h"
#include "Checkbox.h"

class Looper;

class Rewriter : public IAudioProcessor, public IDrawableModule, public IButtonListener
{
public:
   Rewriter();
   virtual ~Rewriter();
   static IDrawableModule* Create() { return new Rewriter(); }
   
   string GetTitleLabel() override { return "looper rewriter"; }
   void CreateUIControls() override;

   void Go();
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IAudioSource
   void Process(double time) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;

   double mStartRecordTime;

   ClickButton* mRewriteButton;
   ClickButton* mStartRecordTimeButton;

   RollingBuffer mRecordBuffer;
   Looper* mConnectedLooper;
   
   PatchCableSource* mLooperCable;
};


#endif /* defined(__modularSynth__Rewriter__) */


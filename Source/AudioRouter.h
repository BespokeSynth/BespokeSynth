//
//  AudioRouter.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/7/13.
//
//

#ifndef __modularSynth__AudioRouter__
#define __modularSynth__AudioRouter__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "RadioButton.h"

class AudioRouter : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IRadioButtonListener
{
public:
   AudioRouter();
   virtual ~AudioRouter();
   static IDrawableModule* Create() { return new AudioRouter(); }
   
   string GetTitleLabel() override { return "router"; }
   void CreateUIControls() override;

   void AddReceiver(IAudioReceiver* receiver, const char* name);
   void SetActiveIndex(int index) { mRouteIndex = index; }

   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;

   //IAudioSource
   void Process(double time) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;

   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* button, int oldVal) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int& h) override;
   bool Enabled() const override { return true; }

   int mInputBufferSize;
   float* mInputBuffer;

   int mRouteIndex;
   RadioButton* mRouteSelector;
   vector<IAudioReceiver*> mReceivers;
};


#endif /* defined(__modularSynth__AudioRouter__) */


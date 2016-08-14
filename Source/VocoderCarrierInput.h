//
//  VocoderCarrierInput.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/18/13.
//
//

#ifndef __modularSynth__VocoderCarrierInput__
#define __modularSynth__VocoderCarrierInput__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"

class VocoderBase
{
public:
   virtual ~VocoderBase() {}
   virtual void SetCarrierBuffer(float* buffer, int bufferSize) = 0;
};

class VocoderCarrierInput : public IAudioReceiver, public IAudioSource, public IDrawableModule
{
public:
   VocoderCarrierInput();
   virtual ~VocoderCarrierInput();
   static IDrawableModule* Create() { return new VocoderCarrierInput(); }
   
   string GetTitleLabel() override { return "carrier"; }
   void CreateUIControls() override;

   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;

   //IAudioSource
   void Process(double time) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=60; h=0; }
   bool Enabled() const override { return true; }

   int mInputBufferSize;
   float* mInputBuffer;

   VocoderBase* mVocoder;
};


#endif /* defined(__modularSynth__VocoderCarrierInput__) */

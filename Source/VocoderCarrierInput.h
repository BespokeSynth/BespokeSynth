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
#include "IAudioProcessor.h"
#include "IDrawableModule.h"

class VocoderBase
{
public:
   virtual ~VocoderBase() {}
   virtual void SetCarrierBuffer(float* buffer, int bufferSize) = 0;
};

class VocoderCarrierInput : public IAudioProcessor, public IDrawableModule
{
public:
   VocoderCarrierInput();
   virtual ~VocoderCarrierInput();
   static IDrawableModule* Create() { return new VocoderCarrierInput(); }
   
   string GetTitleLabel() override { return "carrier"; }
   void CreateUIControls() override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=60; h=0; }
   bool Enabled() const override { return true; }

   VocoderBase* mVocoder;
};


#endif /* defined(__modularSynth__VocoderCarrierInput__) */

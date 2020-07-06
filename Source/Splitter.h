/*
  ==============================================================================

    Splitter.h
    Created: 10 Oct 2017 9:50:00pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "Ramp.h"
#include "PatchCableSource.h"

class Splitter : public IAudioProcessor, public IDrawableModule
{
public:
   Splitter();
   virtual ~Splitter();
   static IDrawableModule* Create() { return new Splitter(); }
   
   string GetTitleLabel() override { return "splitter"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return 2; }
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=80; h=10; }
   bool Enabled() const override { return mEnabled; }
   
   RollingBuffer mVizBuffer2;
   PatchCableSource* mPatchCableSource2;
};

/*
  ==============================================================================

    OSCOutput.h
    Created: 27 Sep 2018 9:35:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "TextEntry.h"
#include "Slider.h"

#define OSC_OUTPUT_MAX_PARAMS 50

class OSCOutput : public IDrawableModule, public ITextEntryListener, public IFloatSliderListener
{
public:
   OSCOutput();
   virtual ~OSCOutput();
   static IDrawableModule* Create() { return new OSCOutput(); }
   
   void Init() override;
   void Poll() override;
   string GetTitleLabel() override { return "osc output"; }
   void CreateUIControls() override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   
   char* mLabels[OSC_OUTPUT_MAX_PARAMS];
   list<TextEntry*> mLabelEntry;
   float mParams[OSC_OUTPUT_MAX_PARAMS];
   list<FloatSlider*> mSliders;
   
   OSCSender mOscOut;
};

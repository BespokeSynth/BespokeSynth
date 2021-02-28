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
#include "INoteReceiver.h"

#define OSC_OUTPUT_MAX_PARAMS 50

class OSCOutput : public IDrawableModule, public ITextEntryListener, public IFloatSliderListener, public INoteReceiver
{
public:
   OSCOutput();
   virtual ~OSCOutput();
   static IDrawableModule* Create() { return new OSCOutput(); }
   
   void Init() override;
   void Poll() override;
   string GetTitleLabel() override { return "osc output"; }
   void CreateUIControls() override;

   void SendFloat(string address, float val);
   void SendInt(string address, int val);
   void SendString(string address, string val);

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);
   void SendCC(int control, int value, int voiceIdx = -1) {}
   
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

   string mOscOutAddress;
   TextEntry* mOscOutAddressEntry;
   int mOscOutPort;
   TextEntry* mOscOutPortEntry;

   string mNoteOutLabel;
   TextEntry* mNoteOutLabelEntry;
   
   OSCSender mOscOut;

   float mWidth;
   float mHeight;
};

/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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

#include "juce_osc/juce_osc.h"

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
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
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
   
   juce::OSCSender mOscOut;

   float mWidth;
   float mHeight;
};

/*
  ==============================================================================

    TakeRecorder.h
    Created: 9 Aug 2017 11:31:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"

class TakeRecorder : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener
{
public:
   TakeRecorder();
   virtual ~TakeRecorder();
   static IDrawableModule* Create() { return new TakeRecorder(); }
   
   string GetTitleLabel() override { return "take recorder"; }
   void CreateUIControls() override;
   
   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=120; h=22; }
   bool Enabled() const override { return mEnabled; }
   
   float mStartSeconds;
   FloatSlider* mStartSecondsSlider;
   int mNumBars;
   IntSlider* mNumBarsSlider;
   bool mRecording;
   Checkbox* mRecordingCheckbox;
};

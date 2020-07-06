/*
  ==============================================================================

    PitchPanner.h
    Created: 25 Mar 2018 9:57:23am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class PitchPanner : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   PitchPanner();
   static IDrawableModule* Create() { return new PitchPanner(); }
   
   string GetTitleLabel() override { return "pitch panner"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 40; }
   bool Enabled() const override { return mEnabled; }
   
   int mPitchLeft;
   IntSlider* mPitchLeftSlider;
   int mPitchRight;
   IntSlider* mPitchRightSlider;
};

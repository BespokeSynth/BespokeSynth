/*
  ==============================================================================

    NotePanAlternator.h
    Created: 25 Mar 2018 9:27:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class NotePanAlternator : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   NotePanAlternator();
   static IDrawableModule* Create() { return new NotePanAlternator(); }
   
   string GetTitleLabel() override { return "pan alternator"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 40; }
   bool Enabled() const override { return mEnabled; }
   
   bool mFlip;
   float mPanOne;
   FloatSlider* mPanOneSlider;
   float mPanTwo;
   FloatSlider* mPanTwoSlider;
};

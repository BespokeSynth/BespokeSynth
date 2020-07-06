/*
  ==============================================================================

    NotePanner.h
    Created: 24 Mar 2018 8:18:19pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class NotePanner : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   NotePanner();
   static IDrawableModule* Create() { return new NotePanner(); }
   
   string GetTitleLabel() override { return "note panner"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   float mPan;
   FloatSlider* mPanSlider;
};


/*
  ==============================================================================

    NoteRangeFilter.h
    Created: 29 Jan 2020 9:18:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class NoteRangeFilter : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   NoteRangeFilter();
   static IDrawableModule* Create() { return new NoteRangeFilter(); }
   
   string GetTitleLabel() override { return "note range filter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 38; }
   bool Enabled() const override { return mEnabled; }
   
   int mMinPitch;
   IntSlider* mMinPitchSlider;
   int mMaxPitch;
   IntSlider* mMaxPitchSlider;
};

/*
  ==============================================================================

    NoteChance.h
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "Slider.h"

class NoteChance : public NoteEffectBase, public IFloatSliderListener, public IDrawableModule
{
public:
   NoteChance();
   virtual ~NoteChance();
   static IDrawableModule* Create() { return new NoteChance(); }
   
   string GetTitleLabel() override { return "note chance"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float mChance;
   FloatSlider* mChanceSlider;
   float mLastRejectTime;
   float mLastAcceptTime;
};

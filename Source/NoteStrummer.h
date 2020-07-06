/*
  ==============================================================================

    NoteStrummer.h
    Created: 2 Apr 2018 9:27:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "Slider.h"
#include "Transport.h"

class NoteStrummer : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   NoteStrummer();
   virtual ~NoteStrummer();
   static IDrawableModule* Create() { return new NoteStrummer(); }
   
   string GetTitleLabel() override { return "note strummer"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 200; height = 35; }
   bool Enabled() const override { return true; }
   
   float mStrum;
   float mLastStrumPos;
   FloatSlider* mStrumSlider;
   list<int> mNotes;
};

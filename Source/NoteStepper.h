/*
  ==============================================================================

    NoteStepper.h
    Created: 15 Jul 2021 9:11:21pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class NoteStepper : public INoteReceiver, public INoteSource, public IDrawableModule, public IIntSliderListener
{
public:
   NoteStepper();
   static IDrawableModule* Create() { return new NoteStepper(); }
   
   string GetTitleLabel() override { return "note stepper"; }
   void CreateUIControls() override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return true; }
   
   void SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);

   static const int kMaxDestinations = 16;
   std::array<PatchCableSource*, kMaxDestinations> mDestinationCables;
   float mWidth;
   float mHeight;
   std::array<int, 128> mLastNoteDestinations;
   int mCurrentDestinationIndex;
   int mLength;
   IntSlider* mLengthSlider;
   double mLastNoteOnTime;
};

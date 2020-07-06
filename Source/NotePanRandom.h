/*
  ==============================================================================

    NotePanRandom.h
    Created: 22 Feb 2020 10:39:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class NotePanRandom : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   NotePanRandom();
   static IDrawableModule* Create() { return new NotePanRandom(); }
   
   string GetTitleLabel() override { return "note pan random"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mSpread;
   FloatSlider* mSpreadSlider;
   float mCenter;
   FloatSlider* mCenterSlider;
   
   static const int kPanHistoryDisplaySize = 10;
   struct PanHistoryDisplayItem
   {
      float time;
      float pan;
   };
   PanHistoryDisplayItem mPanHistoryDisplay[kPanHistoryDisplaySize];
   int mPanHistoryDisplayIndex;
   
   float mWidth;
   float mHeight;
};

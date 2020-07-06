//
//  Neighborhooder.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/10/13.
//
//

#ifndef __modularSynth__Neighborhooder__
#define __modularSynth__Neighborhooder__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class Neighborhooder : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   Neighborhooder();
   static IDrawableModule* Create() { return new Neighborhooder(); }
   
   string GetTitleLabel() override { return "notewrap"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = 120; height = 38; }
   bool Enabled() const override { return mEnabled; }

   int mMinPitch;
   int mPitchRange;
   IntSlider* mMinSlider;
   IntSlider* mRangeSlider;
};

#endif /* defined(__modularSynth__Neighborhooder__) */


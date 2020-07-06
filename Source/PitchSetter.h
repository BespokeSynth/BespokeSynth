//
//  PitchAssigner.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/27/15.
//
//

#ifndef __Bespoke__PitchAssigner__
#define __Bespoke__PitchAssigner__

#include <stdio.h>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class PitchSetter : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   PitchSetter();
   static IDrawableModule* Create() { return new PitchSetter(); }
   
   string GetTitleLabel() override { return "pitch"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   int mPitch;
   IntSlider* mPitchSlider;
};

#endif /* defined(__Bespoke__PitchAssigner__) */

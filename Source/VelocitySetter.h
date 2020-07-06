//
//  VelocitySetter.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/16/13.
//
//

#ifndef __modularSynth__VelocitySetter__
#define __modularSynth__VelocitySetter__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class VelocitySetter : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   VelocitySetter();
   static IDrawableModule* Create() { return new VelocitySetter(); }
   
   string GetTitleLabel() override { return "velocity"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 38; }
   bool Enabled() const override { return mEnabled; }
   
   float mVelocity;
   FloatSlider* mVelocitySlider;
   float mRandomness;
   FloatSlider* mRandomnessSlider;
};


#endif /* defined(__modularSynth__VelocitySetter__) */


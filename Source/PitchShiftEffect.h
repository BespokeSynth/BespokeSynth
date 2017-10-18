//
//  PitchShiftEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#ifndef __Bespoke__PitchShiftEffect__
#define __Bespoke__PitchShiftEffect__

#include <iostream>
#include "IAudioEffect.h"
#include "Slider.h"
#include "PitchShifter.h"
#include "RadioButton.h"

class PitchShiftEffect : public IAudioEffect, public IIntSliderListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   PitchShiftEffect();
   
   static IAudioEffect* Create() { return new PitchShiftEffect(); }
   
   string GetTitleLabel() override { return "pitchshift"; }
   void CreateUIControls() override;
   
   //IAudioEffect
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "pitchshift"; }
   
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int& y) override;
   bool Enabled() const override { return mEnabled; }
   
   float mRatio;
   FloatSlider* mRatioSlider;
   int mRatioSelection;
   RadioButton* mRatioSelector;
   PitchShifter mPitchShifter;
};


#endif /* defined(__Bespoke__PitchShiftEffect__) */

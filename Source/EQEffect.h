//
//  EQEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/14.
//
//

#ifndef __Bespoke__EQEffect__
#define __Bespoke__EQEffect__

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "BiquadFilter.h"
#include "RadioButton.h"
#include "Grid.h"
#include "ClickButton.h"

#define NUM_EQ_FILTERS 8

class EQEffect : public IAudioEffect, public IDropdownListener, public IIntSliderListener, public IRadioButtonListener, public IButtonListener, public GridListener
{
public:
   EQEffect();
   ~EQEffect();
   
   static IAudioEffect* Create() { return new EQEffect(); }
   
   string GetTitleLabel() override { return "eq"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   //IAudioEffect
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "eq"; }
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void GridUpdated(Grid* grid, int col, int row, float value, float oldValue) override;
   
private:
   //IDrawableModule
   void GetModuleDimensions(int& width, int& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }

   BiquadFilter mBiquad[NUM_EQ_FILTERS];
   float* mDryBuffer;
   int mDryBufferSize;
   int mNumFilters;
   
   Grid* mMultiSlider;
   ClickButton* mEvenButton;
};

#endif /* defined(__Bespoke__EQEffect__) */

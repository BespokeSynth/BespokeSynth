//
//  LFOController.h
//  modularSynth
//
//  Created by Ryan Challinor on 10/22/13.
//
//

#ifndef __modularSynth__LFOController__
#define __modularSynth__LFOController__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Slider.h"
#include "ClickButton.h"

class FloatSliderLFOControl;
class LFOController;

extern LFOController* TheLFOController;

class LFOController : public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IButtonListener
{
public:
   LFOController();
   ~LFOController();
   static IDrawableModule* Create() { return new LFOController(); }
   static bool CanCreate() { return TheLFOController == nullptr; }
   
   string GetTitleLabel() override { return "LFO control"; }
   void CreateUIControls() override;
   
   void SetSlider(FloatSlider* slider);
   bool WantsBinding(FloatSlider* slider);
   FloatSlider* GetControlled() { return mSlider; }
   
   //IDrawableModule
   void Poll() override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=130; height=77; }
   
   int dummy;
   float dummy2;
   DropdownList* mIntervalSelector;
   DropdownList* mOscSelector;
   FloatSlider* mMinSlider;
   FloatSlider* mMaxSlider;
   bool mWantBind;
   ClickButton* mBindButton;
   double mStopBindTime;
   
   FloatSlider* mSlider;
   FloatSliderLFOControl* mLFO;
};

#endif /* defined(__modularSynth__LFOController__) */


/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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


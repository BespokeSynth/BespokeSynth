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

#pragma once

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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetSlider(FloatSlider* slider);
   bool WantsBinding(FloatSlider* slider);
   FloatSlider* GetControlled() { return mSlider; }

   //IDrawableModule
   void Poll() override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 130;
      height = 77;
   }

   int dummy{ 0 };
   float dummy2{ 0 };
   DropdownList* mIntervalSelector{ nullptr };
   DropdownList* mOscSelector{ nullptr };
   FloatSlider* mMinSlider{ nullptr };
   FloatSlider* mMaxSlider{ nullptr };
   bool mWantBind{ false };
   ClickButton* mBindButton{ nullptr };
   double mStopBindTime{ -1 };

   FloatSlider* mSlider{ nullptr };
   FloatSliderLFOControl* mLFO{ nullptr };
};

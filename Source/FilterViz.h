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
//  FilterViz.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/24/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Slider.h"
#include "ClickButton.h"
#include "IAudioEffect.h"

#define FILTER_VIZ_BINS 1024

class FilterViz : public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IButtonListener
{
public:
   FilterViz();
   ~FilterViz();
   static IDrawableModule* Create() { return new FilterViz(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IDrawableModule
   void Poll() override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   bool IsEnabled() const override { return true; }

private:
   void GraphFilter();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 300;
      height = 200;
   }

   float* mImpulseBuffer{ nullptr };
   float* mFFTOutReal{ nullptr };
   float* mFFTOutImag{ nullptr };

   bool mNeedUpdate{ true };
   std::vector<IAudioEffect*> mFilters;
};

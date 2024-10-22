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
//  Muter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/26/13.
//
//

#pragma once

#include "IAudioEffect.h"
#include "Checkbox.h"
#include "Ramp.h"
#include "Slider.h"

class Muter : public IAudioEffect, public IFloatSliderListener
{
public:
   Muter();
   virtual ~Muter();

   static IAudioEffect* Create() { return new Muter(); }


   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override {}
   std::string GetType() override { return "muter"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 80;
      h = 38;
   }

   bool mPass{ false };

   Checkbox* mPassCheckbox{ nullptr };
   Ramp mRamp;
   float mRampTimeMs{ 3 };
   FloatSlider* mRampTimeSlider{ nullptr };
};

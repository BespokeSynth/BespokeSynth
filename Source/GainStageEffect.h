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
/*
  ==============================================================================

    GainStage.h
    Created: 24 Apr 2021 3:47:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioEffect.h"
#include "Slider.h"
#include "Checkbox.h"

class GainStageEffect : public IAudioEffect, public IFloatSliderListener
{
public:
   GainStageEffect();
   static IAudioEffect* Create() { return new GainStageEffect(); }


   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   std::string GetType() override { return "gainstage"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 20;
   }

   float mGain{ 1 };
   FloatSlider* mGainSlider{ nullptr };
};

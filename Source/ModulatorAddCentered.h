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

    ModulatorAddCentered.h
    Created: 22 Nov 2017 9:50:16am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;

class ModulatorAddCentered : public IDrawableModule, public IFloatSliderListener, public IModulator
{
public:
   ModulatorAddCentered();
   virtual ~ModulatorAddCentered();
   static IDrawableModule* Create() { return new ModulatorAddCentered(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }

   FloatSlider* GetTarget() { return GetSliderTarget(); }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 106;
      h = 17 * 3 + 4;
   }

   float mValue1{ 0 };
   float mValue2{ 0 };
   float mValue2Range{ 1 };

   FloatSlider* mValue1Slider{ nullptr };
   FloatSlider* mValue2Slider{ nullptr };
   FloatSlider* mValue2RangeSlider{ nullptr };
};

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
//  DopplerShift.h
//  Bespoke
//
//  Created by Andrius Merkys on 3/26/25.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "IModulator.h"

class PatchCableSource;

class DopplerShift : public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IModulator
{
public:
   DopplerShift();
   virtual ~DopplerShift();
   static IDrawableModule* Create() { return new DopplerShift(); }

   void CreateUIControls() override;

   void ButtonClicked(ClickButton* button, double time) override;

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
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 146;
      height = 17 * 3 + 4;
   }

   ClickButton* mReverseButton{ nullptr };

   float mSpeedOfSound{ 343 };
   float mSpeed1{ 0 };
   float mSpeed2{ 0 };

   FloatSlider* mSpeed1Slider{ nullptr };
   FloatSlider* mSpeed2Slider{ nullptr };
};

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
//  EQEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/14.
//
//

#pragma once

#include "IAudioEffect.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Slider.h"
#include "BiquadFilter.h"
#include "RadioButton.h"
#include "UIGrid.h"
#include "ClickButton.h"

#define NUM_EQ_FILTERS 8

class EQEffect : public IAudioEffect, public IDropdownListener, public IIntSliderListener, public IRadioButtonListener, public IButtonListener, public UIGridListener
{
public:
   EQEffect();
   ~EQEffect();

   static IAudioEffect* Create() { return new EQEffect(); }

   void CreateUIControls() override;
   void Init() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "basiceq"; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   bool IsEnabled() const override { return mEnabled; }

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   struct FilterBank
   {
      BiquadFilter mBiquad[NUM_EQ_FILTERS]{};
   };

   FilterBank mBanks[ChannelBuffer::kMaxNumChannels]{};
   int mNumFilters{ NUM_EQ_FILTERS };

   UIGrid* mMultiSlider{ nullptr };
   ClickButton* mEvenButton{ nullptr };
};

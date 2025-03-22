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
//  SlowLayers.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/13/15.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "Slider.h"
#include "DropdownList.h"

class SlowLayers : public IAudioProcessor, public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener, public IRadioButtonListener
{
public:
   SlowLayers();
   virtual ~SlowLayers();
   static IDrawableModule* Create() { return new SlowLayers(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Clear();
   int NumBars() { return mNumBars; }
   void SetNumBars(int numBars);

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   bool CheckNeedsDraw() override { return true; }

   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   int LoopLength() const;

   static const int BUFFER_X = 4;
   static const int BUFFER_Y = 4;
   static const int BUFFER_W = 155;
   static const int BUFFER_H = 93;

   float* mBuffer{ nullptr };
   float mLoopPos{ 0 };
   int mNumBars{ 1 };
   float mVol{ 1 };
   float mFeedIn{ 1 };
   float mSmoothedVol{ 1 };
   FloatSlider* mVolSlider{ nullptr };
   ClickButton* mClearButton{ nullptr };
   DropdownList* mNumBarsSelector{ nullptr };
   FloatSlider* mFeedInSlider{ nullptr };
};

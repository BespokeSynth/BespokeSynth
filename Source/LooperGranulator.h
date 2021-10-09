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

    LooperGranulator.h
    Created: 13 Mar 2021 1:55:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "FloatSliderLFOControl.h"
#include "Slider.h"
#include "Ramp.h"
#include "DropdownList.h"
#include "Granulator.h"

class Looper;

class LooperGranulator : public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   LooperGranulator();
   virtual ~LooperGranulator();
   static IDrawableModule* Create() { return new LooperGranulator(); }

   
   void CreateUIControls() override;

   void ProcessFrame(double time, float bufferOffset, float* output);
   void DrawOverlay(ofRectangle bufferRect, int loopLength);
   bool IsActive() { return mOn; }
   bool ShouldFreeze() { return mOn && mFreeze; }
   void OnCommit();

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;

   float mWidth;
   float mHeight;
   PatchCableSource* mLooperCable;
   Looper* mLooper;
   bool mOn;
   Checkbox* mOnCheckbox;
   Granulator mGranulator;
   FloatSlider* mGranOverlap;
   FloatSlider* mGranSpeed;
   FloatSlider* mGranLengthMs;
   float mDummyPos;
   FloatSlider* mPosSlider;
   bool mFreeze;
   Checkbox* mFreezeCheckbox;
   FloatSlider* mGranPosRandomize;
   FloatSlider* mGranSpeedRandomize;
   FloatSlider* mGranSpacingRandomize;
   Checkbox* mGranOctaveCheckbox;
   FloatSlider* mGranWidthSlider;
};

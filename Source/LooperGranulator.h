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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void ProcessFrame(double time, float bufferOffset, float* output);
   void DrawOverlay(ofRectangle bufferRect, int loopLength);
   bool IsActive() { return mOn; }
   bool ShouldFreeze() { return mOn && mFreeze; }
   void OnCommit();

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;

   float mWidth{ 200 };
   float mHeight{ 20 };
   PatchCableSource* mLooperCable{ nullptr };
   Looper* mLooper{ nullptr };
   bool mOn{ false };
   Checkbox* mOnCheckbox{ nullptr };
   Granulator mGranulator;
   FloatSlider* mGranOverlap{ nullptr };
   FloatSlider* mGranSpeed{ nullptr };
   FloatSlider* mGranLengthMs{ nullptr };
   float mDummyPos{ 0 };
   FloatSlider* mPosSlider{ nullptr };
   bool mFreeze{ false };
   Checkbox* mFreezeCheckbox{ nullptr };
   FloatSlider* mGranPosRandomize{ nullptr };
   FloatSlider* mGranSpeedRandomize{ nullptr };
   FloatSlider* mGranSpacingRandomize{ nullptr };
   Checkbox* mGranOctaveCheckbox{ nullptr };
   FloatSlider* mGranWidthSlider{ nullptr };
};

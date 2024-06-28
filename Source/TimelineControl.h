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
//  TimelineControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#pragma once

#include "IDrawableModule.h"
#include "Slider.h"
#include "TextEntry.h"
#include "ClickButton.h"

class TimelineControl : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   TimelineControl();
   ~TimelineControl();
   static IDrawableModule* Create() { return new TimelineControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void ButtonClicked(ClickButton* button, double time) override;

   bool HasTitleBar() const override { return !mDock; }
   bool IsResizable() const override { return !mDock; }
   void Resize(float width, float height) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;

   float GetSliderWidth() { return mWidth - 6; }

   float mWidth{ 400 };
   int mNumMeasures{ 32 };
   TextEntry* mNumMeasuresEntry{ nullptr };
   float mTime{ 0 };
   FloatSlider* mTimeSlider{ nullptr };
   ClickButton* mResetButton{ nullptr };
   bool mLoop{ false };
   Checkbox* mLoopCheckbox{ nullptr };
   int mLoopStart{ 0 };
   IntSlider* mLoopStartSlider{ nullptr };
   int mLoopEnd{ 8 };
   IntSlider* mLoopEndSlider{ nullptr };
   bool mDock{ false };
   Checkbox* mDockCheckbox{ nullptr };
};

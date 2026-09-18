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
//  TableModulator.h
//  Bespoke
//
//  A step-table modulator. Holds a table of integer values (1-100) that you draw in, steps through
//  it at a chosen transport rate (16n / 8n / ...), and outputs the current value as a modulation
//  signal (patch the cable to any slider - great for stepped pitch modulation). The output value is
//  the base value put through simple integer math: (value * multiply + add - subtract) / divide.
//  A randomize button fills the table either truly randomly or deterministically from a seed.
//

#pragma once

#include "IDrawableModule.h"
#include "IModulator.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include <array>

class FileStreamOut;
class FileStreamIn;

class PatchCableSource;

class TableModulator : public IDrawableModule, public IModulator, public ITimeListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener
{
public:
   TableModulator();
   virtual ~TableModulator();
   static IDrawableModule* Create() { return new TableModulator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return true; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IDrawableModule
   void OnClicked(float x, float y, bool right) override;
   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //persist the drawn table (sliders auto-save, but the table values do not)
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void UpdateTransportListener();
   int ComputeOutput(int step) const; //apply the integer math to a table entry
   void Randomize();
   void EditCellAt(float localX, float localY); //set a table cell from a mouse position in the grid

   static const int kMaxSteps = 32;
   std::array<float, kMaxSteps> mTable; //bar heights, 0..1 (mapped into [low,high] at output)

   int mNumSteps{ 8 };
   int mMultiply{ 1 };
   int mAdd{ 0 };
   int mSubtract{ 0 };
   int mDivide{ 1 };
   int mOutLow{ 1 }; //output is clamped to [low, high]
   int mOutHigh{ 100 };

   NoteInterval mInterval{ kInterval_16n };
   int mCurStep{ 0 };
   int mCurrentOutput{ 0 };

   IntSlider* mNumStepsSlider{ nullptr };
   IntSlider* mMultiplySlider{ nullptr };
   IntSlider* mAddSlider{ nullptr };
   IntSlider* mSubtractSlider{ nullptr };
   IntSlider* mDivideSlider{ nullptr };
   IntSlider* mLowSlider{ nullptr };
   IntSlider* mHighSlider{ nullptr };
   ClickButton* mRandomizeButton{ nullptr };
   DropdownList* mIntervalSelector{ nullptr };

   //grid editing
   bool mEditingGrid{ false };
   float mDragOffX{ 0 };
   float mDragOffY{ 0 };

   float mWidth{ 372 };
   float mHeight{ 200 };
};

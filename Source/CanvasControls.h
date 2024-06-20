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
//  CanvasControls.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "DropdownList.h"

class Canvas;
class CanvasElement;

class CanvasControls : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public ITextEntryListener, public IDropdownListener
{
public:
   CanvasControls();
   ~CanvasControls();
   static IDrawableModule* Create() { return new CanvasControls(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   bool HasTitleBar() const override { return false; }
   bool CanMinimize() override { return false; }

   void SetCanvas(Canvas* canvas);
   void SetElement(CanvasElement* element);

   void AllowDragModeSelection(bool allow);

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   bool CanModuleTypeSaveState() const override { return false; }

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   float mWidth{ 200 };
   Canvas* mCanvas{ nullptr };
   CanvasElement* mSelectedElement{ nullptr };
   ClickButton* mRemoveElementButton{ nullptr };
   TextEntry* mNumVisibleRowsEntry{ nullptr };
   ClickButton* mClearButton{ nullptr };
   float mDummyFloat{ 0 };
   int mDummyInt{ 0 };
   DropdownList* mDragModeSelector{ nullptr };
};

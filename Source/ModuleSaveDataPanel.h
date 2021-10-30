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
//  ModuleSaveDataPanel.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/28/14.
//
//

#ifndef __modularSynth__ModuleSaveDataPanel__
#define __modularSynth__ModuleSaveDataPanel__

#include <iostream>
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "TextEntry.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "ModuleSaveData.h"

class ModuleSaveDataPanel;

extern ModuleSaveDataPanel* TheSaveDataPanel;

class ModuleSaveDataPanel : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IDropdownListener, public IButtonListener
{
public:
   ModuleSaveDataPanel();
   ~ModuleSaveDataPanel();
   static IDrawableModule* Create() { return new ModuleSaveDataPanel(); }
   static bool CanCreate() { return TheSaveDataPanel == nullptr; }
   
   std::string GetTitleLabel() const override { return ""; }
   bool AlwaysOnTop() override { return true; }
   bool CanMinimize() override { return false; }
   bool IsSingleton() const override { return true; }
   
   void SetModule(IDrawableModule* module);
   IDrawableModule* GetModule() { return mSaveModule; }
   void UpdatePosition();
   void ReloadSaveData();
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   bool IsSaveable() override { return false; }
   
private:
   void ApplyChanges();
   void FillDropdownList(DropdownList* list, ModuleSaveData::SaveVal* save);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   
   IDrawableModule* mSaveModule;
   std::vector<IUIControl*> mSaveDataControls;
   std::vector<std::string> mLabels;
   ClickButton* mApplyButton;
   ClickButton* mDeleteButton;
   Checkbox* mDrawDebugCheckbox;
   ClickButton* mResetSequencerButton;
   std::map<DropdownList*,ModuleSaveData::SaveVal*> mStringDropdowns;

   int mHeight;
   float mAppearAmount;
   float mAlignmentX;
};

#endif /* defined(__modularSynth__ModuleSaveDataPanel__) */

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
   
   string GetTitleLabel() override { return ""; }
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
   vector<IUIControl*> mSaveDataControls;
   vector<string> mLabels;
   ClickButton* mApplyButton;
   ClickButton* mDeleteButton;
   Checkbox* mDrawDebugCheckbox;
   map<DropdownList*,ModuleSaveData::SaveVal*> mStringDropdowns;

   int mHeight;
   float mAppearAmount;
   float mAlignmentX;
};

#endif /* defined(__modularSynth__ModuleSaveDataPanel__) */

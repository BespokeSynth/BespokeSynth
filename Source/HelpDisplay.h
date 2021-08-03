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
//  HelpDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/15.
//
//

#ifndef __Bespoke__HelpDisplay__
#define __Bespoke__HelpDisplay__

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "ClickButton.h"

class HelpDisplay : public IDrawableModule, public IRadioButtonListener, public IButtonListener
{
public:
   HelpDisplay();
   virtual ~HelpDisplay();
   static IDrawableModule* Create() { return new HelpDisplay(); }
   
   string GetTitleLabel() override { return "help"; }
   bool IsSaveable() override { return false; }
   bool HasTitleBar() const override { return false; }
   void CreateUIControls() override;

   string GetUIControlTooltip(IUIControl* control);
   string GetModuleTooltip(IDrawableModule* module);
   string GetModuleTooltipFromName(string moduleTypeName);

   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override {}
   void ButtonClicked(ClickButton* button) override;

   void ScreenshotModule(IDrawableModule* module);

   static bool sShowTooltips;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;

   void RenderScreenshot(int x, int y, int width, int height, string filename);
   
   struct UIControlTooltipInfo
   {
      string controlName;
      string tooltip;
   };

   struct ModuleTooltipInfo
   {
      string module;
      string tooltip;
      list<UIControlTooltipInfo> controlTooltips;
   };

   void LoadHelp();
   void LoadTooltips();
   ModuleTooltipInfo* FindModuleInfo(string moduleTypeName);
   UIControlTooltipInfo* FindControlInfo(IUIControl* control);
   
   vector<string> mHelpText;
   Checkbox* mShowTooltipsCheckbox;
   ClickButton* mDumpModuleInfoButton;
   ClickButton* mDoModuleScreenshotsButton;
   ClickButton* mDoModuleDocumentationButton;
   ClickButton* mTutorialVideoLinkButton;
   ClickButton* mDocsLinkButton;
   ClickButton* mDiscordLinkButton;
   float mWidth;
   float mHeight;
   static bool sTooltipsLoaded;
   static list<ModuleTooltipInfo> sTooltips;

   list<string> mScreenshotsToProcess;
   IDrawableModule* mScreenshotModule;
};

#endif /* defined(__Bespoke__HelpDisplay__) */

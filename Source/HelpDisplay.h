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

   static bool sShowTooltips;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   
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
   ClickButton* mDumpModuleInfo;
   ClickButton* mTutorialVideoLinkButton;
   float mWidth;
   float mHeight;
   static bool sTooltipsLoaded;
   static list<ModuleTooltipInfo> sTooltips;
};

#endif /* defined(__Bespoke__HelpDisplay__) */

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

class HelpDisplay : public IDrawableModule, public IRadioButtonListener
{
public:
   HelpDisplay();
   virtual ~HelpDisplay();
   static IDrawableModule* Create() { return new HelpDisplay(); }
   
   string GetTitleLabel() override { return "help"; }
   bool IsSaveable() override { return false; }
   bool HasTitleBar() const override { return false; }
   void CreateUIControls() override;

void RadioButtonUpdated(RadioButton* radio, int oldVal) override {}

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   
   void LoadHelp();
   
   string mOverviewText;
   string mModuleReference;
   string mEffectsReference;
   RadioButton* mHelpPageSelector;
   int mHelpPage;
   int mWidth;
   int mHeight;
};

#endif /* defined(__Bespoke__HelpDisplay__) */

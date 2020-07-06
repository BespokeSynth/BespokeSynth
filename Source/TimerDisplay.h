//
//  TimerDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/2/14.
//
//

#ifndef __Bespoke__TimerDisplay__
#define __Bespoke__TimerDisplay__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "ClickButton.h"

class TimerDisplay : public IDrawableModule, public IButtonListener
{
public:
   TimerDisplay();
   ~TimerDisplay();
   static IDrawableModule* Create() { return new TimerDisplay(); }
   
   string GetTitleLabel() override { return "timer"; }
   void CreateUIControls() override;
   
   void ButtonClicked(ClickButton* button) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=150; height=56; }
   
   double mStartTime;
   ClickButton* mResetButton;
};

#endif /* defined(__Bespoke__TimerDisplay__) */


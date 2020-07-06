/*
  ==============================================================================

    PulseButton.h
    Created: 20 Jun 2020 2:46:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "ClickButton.h"

class PulseButton : public IDrawableModule, public IPulseSource, public IButtonListener
{
public:
   PulseButton();
   virtual ~PulseButton();
   static IDrawableModule* Create() { return new PulseButton(); }
   
   string GetTitleLabel() override { return "pulse button"; }
   void CreateUIControls() override;

   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return true; }
   
   ClickButton* mButton;
   float mWidth;
   float mHeight;
};

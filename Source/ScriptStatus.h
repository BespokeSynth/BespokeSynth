/*
  ==============================================================================

    ScriptStatus.h
    Created: 25 Apr 2020 10:51:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "ClickButton.h"

class ScriptStatus : public IDrawableModule, public IButtonListener
{
public:
   ScriptStatus();
   virtual ~ScriptStatus();
   static IDrawableModule* Create() { return new ScriptStatus(); }
   
   void Poll() override;
   
   string GetTitleLabel() override { return "script status"; }
   void CreateUIControls() override;
   
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override { mWidth = w; mHeight = h; }
   
   ClickButton* mResetAll;
   
   string mStatus;
   double mNextUpdateTime;
   
   float mWidth;
   float mHeight;
};

/*
  ==============================================================================

    SampleBrowser.h
    Created: 19 Jun 2021 6:46:36pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "Sample.h"
#include "ClickButton.h"

class SampleBrowser : public IDrawableModule, public IButtonListener
{
public:
   SampleBrowser();
   ~SampleBrowser();
   static IDrawableModule* Create() { return new SampleBrowser(); }
   
   string GetTitleLabel() override { return "sample browser"; }
   
   void CreateUIControls() override;
   
   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=300; height=38+(int)mButtons.size()*17; }
   
   void SetDirectory(String dirPath);
   int GetNumPages() const;
   void ShowPage(int page);
   
   String mCurrentDirectory;
   StringArray mDirectoryListing;
   std::array<ClickButton*, 30> mButtons;
   ClickButton* mBackButton;
   ClickButton* mForwardButton;
   int mCurrentPage;
};


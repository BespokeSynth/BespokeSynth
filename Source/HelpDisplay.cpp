//
//  HelpDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/15.
//
//

#include "HelpDisplay.h"

#include "SynthGlobals.h"
#include "ModularSynth.h"

HelpDisplay::HelpDisplay()
: mHelpPageSelector(nullptr)
, mHelpPage(0)
, mWidth(700)
, mHeight(700)
{
   LoadHelp();
}

void HelpDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mHelpPageSelector = new RadioButton(this,"help page",4,22,&mHelpPage,kRadioHorizontal);
   mHelpPageSelector->AddLabel("overview", 0);
   mHelpPageSelector->AddLabel("modules", 1);
   mHelpPageSelector->AddLabel("effects", 2);
}

HelpDisplay::~HelpDisplay()
{
}

void HelpDisplay::LoadHelp()
{
   File file(ofToDataPath("help.txt").c_str());
   if (file.existsAsFile())
   {
      juce::String contents = file.loadFileAsString();
      vector<string> tokens = ofSplitString(contents.toStdString(), "#");
      if (tokens.size() == 3)
      {
         mOverviewText = tokens[0];
         mModuleReference = tokens[1];
         mEffectsReference = tokens[2];
      }
   }
}

void HelpDisplay::DrawModule()
{
   ofPushStyle();
   ofSetColor(50,50,50,200);
   ofFill();
   ofRect(0,0,mWidth,mHeight);
   ofPopStyle();
   
   mHelpPageSelector->Draw();
   
   ofRectangle rect;
   if (mHelpPage == 0)
      rect = gFont.DrawStringWrap(mOverviewText,15,4,55,mWidth-8);
   if (mHelpPage == 1)
      rect = gFont.DrawStringWrap(mModuleReference,15,4,55,mWidth-8);
   if (mHelpPage == 2)
      rect = gFont.DrawStringWrap(mEffectsReference,15,4,55,mWidth-8);
   
   mHeight = rect.height + 55;
}

void HelpDisplay::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

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
: mHelpPageSelector(NULL)
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
   //TODO_PORT(Ryan)
   return;
   
   File file(ofToDataPath("help.txt").c_str());
   FileInputStream fin(file);
   
   if (fin.openedOk())
   {
      //getline (fin,mOverviewText,'#');
      //getline (fin,mModuleReference,'#');
      //getline (fin,mEffectsReference,'#');
      //fin.close();
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

void HelpDisplay::GetModuleDimensions(int& w, int& h)
{
   w = mWidth;
   h = mHeight;
}

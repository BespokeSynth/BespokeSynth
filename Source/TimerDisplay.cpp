//
//  TimerDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 7/2/14.
//
//

#include "TimerDisplay.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

TimerDisplay::TimerDisplay()
: mResetButton(nullptr)
{
   mStartTime = gTime;
}

void TimerDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mResetButton = new ClickButton(this,"reset",15,39);
}

TimerDisplay::~TimerDisplay()
{
}

void TimerDisplay::ButtonClicked(ClickButton* button)
{
   if (button == mResetButton)
      mStartTime = gTime;
}

void TimerDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mResetButton->Draw();
   
   int secs = (int)((gTime-mStartTime) / 1000);
   int mins = secs / 60;
   secs %= 60;
   
   string zeroPadMins = "";
   if (mins < 10)
      zeroPadMins = "0";
   
   string zeroPadSecs = "";
   if (secs < 10)
      zeroPadSecs = "0";
   
   ofPushStyle();
   ofSetColor(255,255,255);
   gFont.DrawString(zeroPadMins+ofToString(mins)+":"+zeroPadSecs+ofToString(secs),54, 15,36);
   ofPopStyle();
}


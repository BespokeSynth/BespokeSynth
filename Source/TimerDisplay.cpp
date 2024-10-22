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
{
   mStartTime = gTime;
}

void TimerDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mResetButton = new ClickButton(this, "reset", 15, 39);
}

TimerDisplay::~TimerDisplay()
{
}

void TimerDisplay::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
      mStartTime = gTime;
}

void TimerDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mResetButton->Draw();

   int secs = (int)((gTime - mStartTime) / 1000);
   int mins = secs / 60;
   secs %= 60;

   std::string zeroPadMins = "";
   if (mins < 10)
      zeroPadMins = "0";

   std::string zeroPadSecs = "";
   if (secs < 10)
      zeroPadSecs = "0";

   ofPushStyle();
   ofSetColor(255, 255, 255);
   gFont.DrawString(zeroPadMins + ofToString(mins) + ":" + zeroPadSecs + ofToString(secs), 52, 15, 36);
   ofPopStyle();
}

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
//  TimerDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 7/2/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "ClickButton.h"

class TimerDisplay : public IDrawableModule, public IButtonListener
{
public:
   TimerDisplay();
   ~TimerDisplay();
   static IDrawableModule* Create() { return new TimerDisplay(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void ButtonClicked(ClickButton* button, double time) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 150;
      height = 56;
   }

   double mStartTime{ 0 };
   ClickButton* mResetButton{ nullptr };
};

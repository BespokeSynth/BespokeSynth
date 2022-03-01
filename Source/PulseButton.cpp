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
/*
  ==============================================================================

    PulseButton.cpp
    Created: 20 Jun 2020 2:46:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseButton.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "Transport.h"

PulseButton::PulseButton()
{
}

PulseButton::~PulseButton()
{
}

void PulseButton::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   BUTTON(mButton,"pulse");
   ENDUIBLOCK(mWidth,mHeight);
}

void PulseButton::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mButton->Draw();
}

void PulseButton::ButtonClicked(ClickButton* button)
{
   if (button == mButton)
   {
      double time = gTime + TheTransport->GetEventLookaheadMs();
      if (mForceImmediate)
         time = gTime;
      DispatchPulse(GetPatchCableSource(), time, 1, 0);
   }
}

void PulseButton::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("force_immediate", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseButton::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mForceImmediate = mModuleSaveData.GetBool("force_immediate");
}

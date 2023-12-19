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
//  PitchToValue.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#include "PitchToValue.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"
#include "UIControlMacros.h"

PitchToValue::PitchToValue()
: mControlCable(nullptr)
, mValue(0)
, mValueEntry(nullptr)
{
}

PitchToValue::~PitchToValue()
{
}

void PitchToValue::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UICONTROL_CUSTOM(mValueEntry, new TextEntry(UICONTROL_BASICS("value"),7,&mValue,-99999,99999); mValueEntry->DrawLabel(true););
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mButton, new ClickButton(UICONTROL_BASICS("set")));
   ENDUIBLOCK(mWidth, mHeight);
   
   mControlCable = new PatchCableSource(this, kConnectionType_Modulator);
   AddPatchCableSource(mControlCable);
}

void PitchToValue::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValueEntry->Draw();
   mButton->Draw();
}

void PitchToValue::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void PitchToValue::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled)
   {
      Go();
   }
}

void PitchToValue::ButtonClicked(ClickButton* button)
{
   if (button == mButton)
      Go();
}

void PitchToValue::Go()
{
   if (mTarget)
   {
      mTarget->SetValue(mValue);
      mControlCable->AddHistoryEvent(gTime, true);
      mControlCable->AddHistoryEvent(gTime + 15, false);
   }
}

void PitchToValue::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   std::string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void PitchToValue::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchToValue::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);
}

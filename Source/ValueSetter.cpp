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
//  ValueSetter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#include "ValueSetter.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"
#include "UIControlMacros.h"

ValueSetter::ValueSetter()
: mControlCable(nullptr)
, mValue(0)
, mValueEntry(nullptr)
{
}

ValueSetter::~ValueSetter()
{
}

void ValueSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UICONTROL_CUSTOM(mValueEntry, new TextEntry(UICONTROL_BASICS("value"),7,&mValue,-99999,99999); mValueEntry->DrawLabel(true););
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mButton, new ClickButton(UICONTROL_BASICS("set")));
   ENDUIBLOCK(mWidth, mHeight);

   auto entryRect = mValueEntry->GetRect();
   mValueSlider = new FloatSlider(this, "slider", entryRect.x, entryRect.y, entryRect.width, entryRect.height, &mValue, 0, 1);
   mValueSlider->SetShowing(false);
   
   mControlCable = new PatchCableSource(this, kConnectionType_Modulator);
   AddPatchCableSource(mControlCable);
}

void ValueSetter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValueEntry->Draw();
   mValueSlider->Draw();
   mButton->Draw();
}

void ValueSetter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void ValueSetter::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled)
   {
      Go();
   }
}

void ValueSetter::ButtonClicked(ClickButton* button)
{
   if (button == mButton)
      Go();
}

void ValueSetter::Go()
{
   if (mTarget)
   {
      mTarget->SetValue(mValue);
      mControlCable->AddHistoryEvent(gTime, true);
      mControlCable->AddHistoryEvent(gTime + 15, false);
   }
}

void ValueSetter::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   std::string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ValueSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("show_slider", moduleInfo, false);
   
   SetUpFromSaveData();
}

void ValueSetter::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);

   bool showSlider = mModuleSaveData.GetBool("show_slider");
   mValueEntry->SetShowing(!showSlider);
   mValueSlider->SetShowing(showSlider);
}

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
{
}

ValueSetter::~ValueSetter()
{
}

void ValueSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UICONTROL_CUSTOM(mValueEntry, new TextEntry(UICONTROL_BASICS("value"), 7, &mValue, -99999, 99999); mValueEntry->DrawLabel(true););
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mButton, new ClickButton(UICONTROL_BASICS("set")));
   ENDUIBLOCK(mWidth, mHeight);

   auto entryRect = mValueEntry->GetRect();
   mValueSlider = new FloatSlider(this, "slider", entryRect.x, entryRect.y, entryRect.width, entryRect.height, &mValue, 0, 1);
   mValueSlider->SetShowing(false);

   mControlCable = new PatchCableSource(this, kConnectionType_ValueSetter);
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
   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      if (i < mControlCable->GetPatchCables().size())
      {
         mTargets[i] = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[i]->GetTarget());
         if (mControlCable->GetPatchCables().size() == 1 && mTargets[i] != nullptr)
            mValueSlider->SetExtents(mTargets[i]->GetModulationRangeMin(), mTargets[i]->GetModulationRangeMax());
      }
      else
      {
         mTargets[i] = nullptr;
      }
   }
}

void ValueSetter::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled)
   {
      ComputeSliders((time - gTime) * gSampleRateMs);
      Go(time);
   }
}

void ValueSetter::ButtonClicked(ClickButton* button, double time)
{
   if (mEnabled && button == mButton && mLastClickTime != time)
   {
      mLastClickTime = time;
      Go(time);
   }
}

void ValueSetter::Go(double time)
{
   mControlCable->AddHistoryEvent(time, true);
   mControlCable->AddHistoryEvent(time + 15, false);

   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      if (mTargets[i] != nullptr)
         mTargets[i]->SetValue(mValue, time, K(forceUpdate));
   }
}

void ValueSetter::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ValueSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("show_slider", moduleInfo, false);

   SetUpFromSaveData();
}

void ValueSetter::SetUpFromSaveData()
{
   bool showSlider = mModuleSaveData.GetBool("show_slider");
   mValueEntry->SetShowing(!showSlider);
   mValueSlider->SetShowing(showSlider);
}

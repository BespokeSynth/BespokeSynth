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
//  Selector.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#include "Selector.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

Selector::Selector()
{
}

Selector::~Selector()
{
}

void Selector::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mSelector = new RadioButton(this,"selector",3,3,&mCurrentValue);
}

void Selector::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<mControlCables.size(); ++i)
   {
      mControlCables[i]->SetManualPosition(GetRect(true).width - 5, 3+RadioButton::GetSpacing()*(i+.5f));
   }
   
   mSelector->Draw();
}

void Selector::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (!fromUserClick)
      return;

   for (int i=0; i<mControlCables.size(); ++i)
   {
      if (mControlCables[i] == cableSource)
      {
         if (i == mControlCables.size() - 1)
         {
            if (cableSource->GetTarget())
            {
               PatchCableSource* cable = new PatchCableSource(this, kConnectionType_Modulator);
               AddPatchCableSource(cable);
               mControlCables.push_back(cable);
            }
         }
         else if (cableSource->GetTarget() == nullptr)
         {
            RemoveFromVector(cableSource, mControlCables);
         }
         
         SyncList();
         
         break;
      }
   }
}

void Selector::SyncList()
{
   mSelector->Clear();
   for (int i=0; i<mControlCables.size()-1; ++i)
   {
      std::string controlName = "";
      if (mControlCables[i]->GetTarget())
         controlName = mControlCables[i]->GetTarget()->Path();
      mSelector->AddLabel(controlName.c_str(), i);
   }
}

void Selector::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int range = (int)mControlCables.size() - 1;
   if (velocity > 0 && range > 0)
      SetIndex(pitch % range);
}

void Selector::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   SetIndex(mCurrentValue);
}

void Selector::SetIndex(int index)
{
   mCurrentValue = index;

   IUIControl* controlToEnable = nullptr;
   for (int i=0; i<mControlCables.size(); ++i)
   {
      IUIControl* uicontrol = nullptr;
      if (mControlCables[i]->GetTarget())
         uicontrol = dynamic_cast<IUIControl*>(mControlCables[i]->GetTarget());
      if (uicontrol)
      {
         if (mCurrentValue == i)
            controlToEnable = uicontrol;
         else
            uicontrol->SetValue(0);
      }
   }
   
   if (controlToEnable)
      controlToEnable->SetValue(1);
}

namespace
{
   const float extraW = 20;
   const float extraH = 6 + RadioButton::GetSpacing();
}

void Selector::GetModuleDimensions(float& width, float& height)
{
   width = mSelector->GetRect().width + extraW;
   height = mSelector->GetRect().height + extraH;
}

void Selector::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["uicontrols"].resize((unsigned int)mControlCables.size()-1);
   for (int i=0; i<mControlCables.size()-1; ++i)
   {
      std::string controlName = "";
      if (mControlCables[i]->GetTarget())
         controlName = mControlCables[i]->GetTarget()->Path();
      moduleInfo["uicontrols"][i] = controlName;
   }
}

void Selector::LoadLayout(const ofxJSONElement& moduleInfo)
{
   const Json::Value& controls = moduleInfo["uicontrols"];
   
   for (int i=0; i<controls.size(); ++i)
   {
      std::string controlPath = controls[i].asString();
      IUIControl* control = nullptr;
      if (!controlPath.empty())
         control = TheSynth->FindUIControl(controlPath);
      PatchCableSource* cable = new PatchCableSource(this, kConnectionType_Modulator);
      AddPatchCableSource(cable);
      cable->SetTarget(control);
      mControlCables.push_back(cable);
   }
   
   //add extra cable
   PatchCableSource* cable = new PatchCableSource(this, kConnectionType_Modulator);
   AddPatchCableSource(cable);
   mControlCables.push_back(cable);
   
   SyncList();
   
   SetUpFromSaveData();
}

void Selector::SetUpFromSaveData()
{
}

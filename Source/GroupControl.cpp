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
//  GroupControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#include "GroupControl.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

GroupControl::GroupControl()
{
}

GroupControl::~GroupControl()
{
}

void GroupControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGroupCheckbox = new Checkbox(this,"group enabled",3,3,&mGroupEnabled);
}

void GroupControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<mControlCables.size(); ++i)
   {
      mControlCables[i]->SetManualPosition(10 + i * 12, 25);
   }
   
   mGroupCheckbox->Draw();
}

void GroupControl::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
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
         
         break;
      }
   }
}

void GroupControl::CheckboxUpdated(Checkbox* checkbox)
{
   for (int i=0; i<mControlCables.size(); ++i)
   {
      IUIControl* uicontrol = nullptr;
      if (mControlCables[i]->GetTarget())
         uicontrol = dynamic_cast<IUIControl*>(mControlCables[i]->GetTarget());
      if (uicontrol)
         uicontrol->SetValue(mGroupEnabled ? 1 : 0);
   }
}

namespace
{
   const float extraW = 20;
}

void GroupControl::GetModuleDimensions(float& width, float& height)
{
   width = mGroupCheckbox->GetRect().width + extraW;
   height = 38;
}

void GroupControl::SaveLayout(ofxJSONElement& moduleInfo)
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

void GroupControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   const Json::Value& controls = moduleInfo["uicontrols"];
   
   for (int i=0; i<controls.size(); ++i)
   {
      try
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
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }
   
   //add extra cable
   PatchCableSource* cable = new PatchCableSource(this, kConnectionType_Modulator);
   AddPatchCableSource(cable);
   mControlCables.push_back(cable);
   
   SetUpFromSaveData();
}

void GroupControl::SetUpFromSaveData()
{
}

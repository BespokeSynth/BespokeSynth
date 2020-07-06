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
               PatchCableSource* cable = new PatchCableSource(this, kConnectionType_UIControl);
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
      string controlName = "";
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
      string controlPath = controls[i].asString();
      IUIControl* control = nullptr;
      if (!controlPath.empty())
         control = TheSynth->FindUIControl(controlPath);
      PatchCableSource* cable = new PatchCableSource(this, kConnectionType_UIControl);
      AddPatchCableSource(cable);
      cable->SetTarget(control);
      mControlCables.push_back(cable);
   }
   
   //add extra cable
   PatchCableSource* cable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(cable);
   mControlCables.push_back(cable);
   
   SetUpFromSaveData();
}

void GroupControl::SetUpFromSaveData()
{
}

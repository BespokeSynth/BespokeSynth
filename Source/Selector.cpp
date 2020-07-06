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
      string controlName = "";
      if (mControlCables[i]->GetTarget())
         controlName = mControlCables[i]->GetTarget()->Path();
      mSelector->AddLabel(controlName.c_str(), i);
   }
}

void Selector::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
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
      string controlName = "";
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
   
   SyncList();
   
   SetUpFromSaveData();
}

void Selector::SetUpFromSaveData()
{
}

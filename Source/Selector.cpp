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

   mSelector = new RadioButton(this, "selector", 3, 3, &mCurrentValue);
}

void Selector::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < (int)mControlCables.size(); ++i)
   {
      mControlCables[i]->SetManualPosition(GetRect(true).width - 5, 3 + RadioButton::GetSpacing() * (i + .5f));
   }

   mSelector->Draw();
}

void Selector::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   SyncList();
}

void Selector::SyncList()
{
   mSelector->Clear();
   for (int i = 0; i < (int)mControlCables.size(); ++i)
   {
      std::string controlName = "";
      if (mControlCables[i]->GetTarget())
         controlName = mControlCables[i]->GetTarget()->Path();
      mSelector->AddLabel(controlName.c_str(), i);
   }
}

void Selector::PlayNote(NoteMessage note)
{
   int range = (int)mControlCables.size() - 1;
   if (note.velocity > 0 && range > 0)
      SetIndex(note.pitch % range, note.time);
}

void Selector::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
   SetIndex(mCurrentValue, time);
}

void Selector::SetIndex(int index, double time)
{
   mCurrentValue = index;

   std::vector<IUIControl*> controlsToEnable;
   for (int i = 0; i < (int)mControlCables.size(); ++i)
   {
      for (auto* cable : mControlCables[i]->GetPatchCables())
      {
         IUIControl* uicontrol = dynamic_cast<IUIControl*>(cable->GetTarget());
         if (uicontrol)
         {
            if (mCurrentValue == i)
               controlsToEnable.push_back(uicontrol);
            else
               uicontrol->SetValue(0, time);
         }
      }
   }

   for (auto* control : controlsToEnable)
      control->SetValue(1, time);
}

namespace
{
   const float extraW = 20;
   const float extraH = 6;
}

void Selector::GetModuleDimensions(float& width, float& height)
{
   width = mSelector->GetRect().width + extraW;
   height = mSelector->GetRect().height + extraH;
}

void Selector::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_items", moduleInfo, 2, 1, 99, K(isTextField));

   if (!moduleInfo["uicontrols"].isNull()) //handling for older revision loading
      mModuleSaveData.SetInt("num_items", (int)moduleInfo["uicontrols"].size());

   SetUpFromSaveData();
}

void Selector::SetUpFromSaveData()
{
   int numItems = mModuleSaveData.GetInt("num_items");
   int oldNumItems = (int)mControlCables.size();
   if (numItems > oldNumItems)
   {
      for (int i = oldNumItems; i < numItems; ++i)
      {
         PatchCableSource* cable = new PatchCableSource(this, kConnectionType_ValueSetter);
         AddPatchCableSource(cable);
         mControlCables.push_back(cable);
      }
   }
   else if (numItems < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= numItems; --i)
      {
         RemovePatchCableSource(mControlCables[i]);
      }
      mControlCables.resize(numItems);
   }

   SyncList();
}

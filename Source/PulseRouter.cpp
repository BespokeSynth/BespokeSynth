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

    PulseRouter.cpp
    Created: 31/03/2024
    Author:  Ryan Challinor / ArkyonVeil

  ==============================================================================
*/


#include "PulseRouter.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "PatchCableSource.h"

PulseRouter::PulseRouter()
{
}

void PulseRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this, "route", 5, 3, &mRouteMask);

   GetPatchCableSource()->SetEnabled(false);
}


void PulseRouter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRouteSelector->Draw();
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      ofVec2f pos = mRouteSelector->GetOptionPosition(i) - mRouteSelector->GetPosition();
      mDestinationCables[i]->SetManualPosition(pos.x + 10, pos.y + 4);
   }
}

void PulseRouter::SetSelectedMask(int mask)
{
   mRouteMask = mask;
}

bool PulseRouter::IsIndexActive(int idx) const
{
   if (mRadioButtonMode)
      return mRouteMask == idx;
   return mRouteMask & (1 << idx);
}

void PulseRouter::OnPulse(double time, float velocity, int flags)
{
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      if (IsIndexActive(i))
      {
         DispatchPulse(mDestinationCables[i], time, velocity, flags);
      }
   }
}

void PulseRouter::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
}

void PulseRouter::Poll()
{
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
      mDestinationCables[i]->SetShowing(!mOnlyShowActiveCables || IsIndexActive(i));
}


void PulseRouter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   //Hopefully this isn't important.
   //INoteSource::PostRepatch(cableSource, fromUserClick);

   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      if (cableSource == mDestinationCables[i])
      {
         IClickable* target = cableSource->GetTarget();
         std::string name = target ? target->Name() : "                      ";
         mRouteSelector->SetLabel(name.c_str(), i);
      }
   }
}

void PulseRouter::GetModuleDimensions(float& width, float& height)
{
   float w, h;
   mRouteSelector->GetDimensions(w, h);
   width = 20 + w;
   height = 8 + h;
}

void PulseRouter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_items", moduleInfo, 2, 1, 99, K(isTextField));
   mModuleSaveData.LoadBool("radiobuttonmode", moduleInfo, true);
   mModuleSaveData.LoadBool("only_show_active_cables", moduleInfo, false);

   SetUpFromSaveData();
}

void PulseRouter::SetUpFromSaveData()
{
   int numItems = mModuleSaveData.GetInt("num_items");
   int oldNumItems = (int)mDestinationCables.size();
   if (numItems > oldNumItems)
   {
      for (int i = oldNumItems; i < numItems; ++i)
      {
         mDestinationCables.push_back(new PatchCableSource(this, kConnectionType_Pulse));
         mRouteSelector->AddLabel("                      ", i);
         AddPatchCableSource(mDestinationCables[i]);
         mDestinationCables[i]->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
         ofRectangle rect = GetRect(true);
         mDestinationCables[i]->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
      }
   }
   else if (numItems < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= numItems; --i)
      {
         mRouteSelector->RemoveLabel(i);
         RemovePatchCableSource(mDestinationCables[i]);
      }
      mDestinationCables.resize(numItems);
   }
   mRadioButtonMode = mModuleSaveData.GetBool("radiobuttonmode");
   mRouteSelector->SetMultiSelect(!mRadioButtonMode);
   mOnlyShowActiveCables = mModuleSaveData.GetBool("only_show_active_cables");
}

void PulseRouter::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["num_items"] = (int)mDestinationCables.size();
}
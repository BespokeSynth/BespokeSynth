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
//  NoteRouter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/24/13.
//
//

#include "NoteRouter.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "PatchCableSource.h"

NoteRouter::NoteRouter()
{
}

void NoteRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this, "route", 5, 3, &mRouteMask);

   GetPatchCableSource()->SetEnabled(false);
}

void NoteRouter::Poll()
{
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
      mDestinationCables[i]->GetPatchCableSource()->SetShowing(!mOnlyShowActiveCables || IsIndexActive(i));
}

void NoteRouter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRouteSelector->Draw();
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      ofVec2f pos = mRouteSelector->GetOptionPosition(i) - mRouteSelector->GetPosition();
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(pos.x + 10, pos.y + 4);
   }
}

void NoteRouter::SetSelectedMask(int mask)
{
   int oldMask = mRouteMask;
   mRouteMask = mask;
   RadioButtonUpdated(mRouteSelector, oldMask, NextBufferTime(false));
}

bool NoteRouter::IsIndexActive(int idx) const
{
   if (mRadioButtonMode)
      return mRouteMask == idx;
   else
      return mRouteMask & (1 << idx);
}

void NoteRouter::PlayNote(NoteMessage note)
{
   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      if (IsIndexActive(i))
      {
         mDestinationCables[i]->PlayNoteOutput(note);
      }
   }
}

void NoteRouter::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
   if (radio == mRouteSelector)
   {
      if (mRadioButtonMode)
      {
         if (oldVal < (int)mDestinationCables.size())
            mDestinationCables[oldVal]->Flush(time);
      }
      else //bitmask mode
      {
         int changed = mRouteMask ^ oldVal;
         int removed = changed & oldVal;

         for (int i = 0; i <= int(log2(removed)); ++i)
         {
            if (1 & (removed >> i))
            {
               if (i < (int)mDestinationCables.size())
                  mDestinationCables[i]->Flush(time);
            }
         }
      }
   }
}

void NoteRouter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   INoteSource::PostRepatch(cableSource, fromUserClick);

   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      if (cableSource == mDestinationCables[i]->GetPatchCableSource())
      {
         IClickable* target = cableSource->GetTarget();
         std::string name = target ? target->Name() : "                      ";
         mRouteSelector->SetLabel(name.c_str(), i);
      }
   }
}

void NoteRouter::GetModuleDimensions(float& width, float& height)
{
   float w, h;
   mRouteSelector->GetDimensions(w, h);
   width = 20 + w;
   height = 8 + h;
}

void NoteRouter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_items", moduleInfo, 2, 1, 99, K(isTextField));
   mModuleSaveData.LoadBool("radiobuttonmode", moduleInfo, true);
   mModuleSaveData.LoadBool("only_show_active_cables", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteRouter::SetUpFromSaveData()
{
   int numItems = mModuleSaveData.GetInt("num_items");
   int oldNumItems = (int)mDestinationCables.size();
   if (numItems > oldNumItems)
   {
      for (int i = oldNumItems; i < numItems; ++i)
      {
         mRouteSelector->AddLabel("                      ", i);
         auto* additionalCable = new AdditionalNoteCable();
         additionalCable->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
         AddPatchCableSource(additionalCable->GetPatchCableSource());
         mDestinationCables.push_back(additionalCable);
      }
   }
   else if (numItems < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= numItems; --i)
      {
         mRouteSelector->RemoveLabel(i);
         RemovePatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      }
      mDestinationCables.resize(numItems);
   }
   mRadioButtonMode = mModuleSaveData.GetBool("radiobuttonmode");
   mRouteSelector->SetMultiSelect(!mRadioButtonMode);
   mOnlyShowActiveCables = mModuleSaveData.GetBool("only_show_active_cables");
}

void NoteRouter::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["num_items"] = (int)mDestinationCables.size();
}

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
//  NoteToggle.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/01/2021
//
//

#include "NoteToggle.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

NoteToggle::NoteToggle()
{
}

NoteToggle::~NoteToggle()
{
}

void NoteToggle::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mControlCable = new PatchCableSource(this, kConnectionType_ValueSetter);
   AddPatchCableSource(mControlCable);
}

void NoteToggle::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToggle::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      if (i < mControlCable->GetPatchCables().size())
         mTargets[i] = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[i]->GetTarget());
      else
         mTargets[i] = nullptr;
   }
}

void NoteToggle::PlayNote(NoteMessage note)
{
   if (note.pitch >= 0 && note.pitch < 128)
      mHeldPitches[note.pitch] = (note.velocity > 0);

   bool hasHeldNotes = false;
   for (int i = 0; i < 128; ++i)
   {
      if (mHeldPitches[i])
         hasHeldNotes = true;
   }

   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      if (mTargets[i] != nullptr)
         mTargets[i]->SetValue(hasHeldNotes ? 1 : 0, note.time);
   }
}

void NoteToggle::GetModuleDimensions(float& width, float& height)
{
   width = 100;
   height = 10;
}

void NoteToggle::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteToggle::SetUpFromSaveData()
{
}

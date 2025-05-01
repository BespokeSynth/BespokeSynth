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
//  VelocityScaler.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/6/16.
//
//

#include "VelocityScaler.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

VelocityScaler::VelocityScaler()
{
}

void VelocityScaler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mScaleSlider = new FloatSlider(this, "scale", 4, 2, 100, 15, &mScale, 0, 2);
}

void VelocityScaler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mScaleSlider->Draw();
}

void VelocityScaler::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      ComputeSliders(0);
      if (note.velocity > 0)
         note.velocity = MAX(1, note.velocity * mScale);
   }

   PlayNoteOutput(note);
}

void VelocityScaler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VelocityScaler::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

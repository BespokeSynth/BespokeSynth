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

    NoteToPulse.cpp
    Created: 20 Sep 2018 9:43:01pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteToPulse.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "ModulationChain.h"

NoteToPulse::NoteToPulse()
{
}

NoteToPulse::~NoteToPulse()
{
}

void NoteToPulse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToPulse::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
      DispatchPulse(GetPatchCableSource(), note.time, note.velocity / 127.0f, 0);
}

void NoteToPulse::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoteToPulse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteToPulse::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

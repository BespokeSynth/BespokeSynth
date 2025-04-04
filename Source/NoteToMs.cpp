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
//  NoteToMs.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/17/15.
//
//

#include "NoteToMs.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

NoteToMs::NoteToMs()
{
}

NoteToMs::~NoteToMs()
{
}

void NoteToMs::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);
}

void NoteToMs::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void NoteToMs::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void NoteToMs::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      mPitch = note.pitch;
      mPitchBend = note.modulation.pitchBend;
   }
}

float NoteToMs::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return 1000 / TheScale->PitchToFreq(mPitch + bend);
}

void NoteToMs::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoteToMs::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteToMs::SetUpFromSaveData()
{
}

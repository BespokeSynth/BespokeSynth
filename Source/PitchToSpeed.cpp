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
 
 PitchToSpeed.cpp
 Created: 28 Nov 2017 9:44:15pm
 Author:  Ryan Challinor
 
 ==============================================================================
 */

#include "PitchToSpeed.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"

PitchToSpeed::PitchToSpeed()
{
}

PitchToSpeed::~PitchToSpeed()
{
}

void PitchToSpeed::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);

   mReferenceFreqSlider = new FloatSlider(this, "ref freq", 3, 2, 100, 15, &mReferenceFreq, 10, 1000);
}

void PitchToSpeed::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mReferenceFreqSlider->Draw();
}

void PitchToSpeed::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void PitchToSpeed::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      mPitch = note.pitch;
      mPitchBend = note.modulation.pitchBend;
   }
}

float PitchToSpeed::Value(int samplesIn)
{
   float bend = mPitchBend ? mPitchBend->GetValue(samplesIn) : 0;
   return TheScale->PitchToFreq(mPitch + bend) / mReferenceFreq;
}

void PitchToSpeed::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void PitchToSpeed::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void PitchToSpeed::SetUpFromSaveData()
{
}

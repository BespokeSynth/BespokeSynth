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
//  VocoderCarrierInput.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/18/13.
//
//

#include "VocoderCarrierInput.h"
#include "Vocoder.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

VocoderCarrierInput::VocoderCarrierInput()
: IAudioProcessor(gBufferSize)
{
}

VocoderCarrierInput::~VocoderCarrierInput()
{
}

void VocoderCarrierInput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   GetPatchCableSource()->AddTypeFilter("fftvocoder");
   GetPatchCableSource()->AddTypeFilter("vocoder");
}

void VocoderCarrierInput::Process(double time)
{
   PROFILER(VocoderCarrierInput);

   if (GetTarget() == nullptr)
      return;

   if (mVocoder == nullptr || GetTarget() != mVocoderTarget)
   {
      mVocoder = dynamic_cast<VocoderBase*>(GetTarget());
      mVocoderTarget = GetTarget();
   }

   if (mVocoder == nullptr)
      return;

   SyncBuffers();

   mVocoder->SetCarrierBuffer(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);

   GetBuffer()->Reset();
}

void VocoderCarrierInput::DrawModule()
{
}

void VocoderCarrierInput::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("vocoder", moduleInfo, "", FillDropdown<VocoderBase*>);

   SetUpFromSaveData();
}

void VocoderCarrierInput::SetUpFromSaveData()
{
   IDrawableModule* vocoder = TheSynth->FindModule(mModuleSaveData.GetString("vocoder"), false);
   mVocoder = dynamic_cast<VocoderBase*>(vocoder);
   GetPatchCableSource()->SetTarget(dynamic_cast<IClickable*>(vocoder));
}

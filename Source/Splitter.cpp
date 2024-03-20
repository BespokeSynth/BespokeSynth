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

    Splitter.cpp
    Created: 10 Oct 2017 9:50:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Splitter.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Splitter::Splitter()
: IAudioProcessor(gBufferSize)
, mVizBuffer2(VIZ_BUFFER_SECONDS * gSampleRate)
{
}

void Splitter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float w, h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w / 2 - 15, h + 3);
   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);

   mPatchCableSource2 = new PatchCableSource(this, kConnectionType_Audio);
   mPatchCableSource2->SetManualPosition(w / 2 + 15, h + 3);
   mPatchCableSource2->SetOverrideVizBuffer(&mVizBuffer2);
   mPatchCableSource2->SetManualSide(PatchCableSource::Side::kBottom);
   AddPatchCableSource(mPatchCableSource2);
}

Splitter::~Splitter()
{
}

void Splitter::Process(double time)
{
   PROFILER(Splitter);

   IAudioReceiver* target0 = GetTarget(0);
   if (target0)
   {
      ChannelBuffer* out = target0->GetBuffer();
      Add(out->GetChannel(0), GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);
   }

   int secondChannel = 1;
   if (GetBuffer()->NumActiveChannels() == 1)
      secondChannel = 0;
   IAudioReceiver* target1 = GetTarget(1);
   if (target1)
   {
      ChannelBuffer* out2 = target1->GetBuffer();
      Add(out2->GetChannel(0), GetBuffer()->GetChannel(secondChannel), GetBuffer()->BufferSize());
      mVizBuffer2.WriteChunk(GetBuffer()->GetChannel(secondChannel), GetBuffer()->BufferSize(), 0);
   }

   GetBuffer()->Reset();
}

void Splitter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void Splitter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("target2", moduleInfo);

   SetUpFromSaveData();
}

void Splitter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IClickable* target2 = TheSynth->FindModule(mModuleSaveData.GetString("target2"));
   if (target2)
      mPatchCableSource2->AddPatchCable(target2);
}

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

    AudioSplitter.cpp
    Created: 26 Aug 2023 7:18:00am
    Author:  Noxy Nixie

  ==============================================================================
*/

#include "AudioSplitter.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

AudioSplitter::AudioSplitter()
: IAudioProcessor(gBufferSize)
{
}

void AudioSplitter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   auto cable = new PatchCableSource(this, kConnectionType_Audio);
   AddPatchCableSource(cable);
   mDestinationCables.push_back(cable);
}

void AudioSplitter::Process(double time)
{
   PROFILER(AudioSplitter);

   SyncBuffers();

   const auto numchannels = GetBuffer()->NumActiveChannels();

   for (const auto cablesource : GetPatchCableSources())
   {
      const auto target = dynamic_cast<IAudioReceiver*>(cablesource->GetTarget());
      if (target)
      {
         ChannelBuffer* out = target->GetBuffer();
         out->SetNumActiveChannels(numchannels);
         for (int ch = 0; ch < numchannels; ++ch)
         {
            Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         }
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void AudioSplitter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   IAudioSource::PostRepatch(cableSource, fromUserClick);

   for (int i = 0; i < mDestinationCables.size(); ++i)
   {
      if (mDestinationCables[i] == cableSource)
      {
         if (i == mDestinationCables.size() - 1)
         {
            if (cableSource->GetTarget())
            {
               auto cable = new PatchCableSource(this, kConnectionType_Audio);
               AddPatchCableSource(cable);
               mDestinationCables.push_back(cable);
            }
         }
         else if (cableSource->GetTarget() == nullptr)
         {
            RemoveFromVector(cableSource, mDestinationCables);
            RemovePatchCableSource(cableSource);
         }

         break;
      }
   }
}


void AudioSplitter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   GetPatchCableSource()->SetManualPosition(20, 12);

   int offset{ 20 };
   for (const auto cablesource : mDestinationCables)
   {
      cablesource->SetManualPosition(offset += 20, 12);
      cablesource->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   }
}

void AudioSplitter::GetModuleDimensions(float& w, float& h)
{
   w = MAX(80, 40 + (20 * mDestinationCables.size()));
   h = 5;
}

void AudioSplitter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void AudioSplitter::SetUpFromSaveData()
{
}

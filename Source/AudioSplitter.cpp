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
}

void AudioSplitter::Process(double time)
{
   PROFILER(AudioSplitter);

   SyncBuffers();

   const auto numchannels = GetBuffer()->NumActiveChannels();

   for (const auto cablesource : mDestinationCables)
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

   int numItems{ 0 };

   const auto maintarget = dynamic_cast<IAudioReceiver*>(GetPatchCableSource()->GetTarget());
   if (maintarget)
       numItems++;

   for (const auto cablesource : mDestinationCables)
   {
      const auto target = dynamic_cast<IAudioReceiver*>(cablesource->GetTarget());
      if (target)
         numItems++;
   }
   int cableSourceCount = static_cast<int>(mDestinationCables.size());
   if (numItems > cableSourceCount)
   {
      for (int i = cableSourceCount; i < numItems; ++i)
      {
         auto* additionalCable = new PatchCableSource(this, kConnectionType_Audio);
         AddPatchCableSource(additionalCable);
         mDestinationCables.push_back(additionalCable);
      }
   }
   else if (numItems < cableSourceCount)
   {
      for (int i = cableSourceCount - 1; i >= numItems; --i)
      {
         const auto target = dynamic_cast<IAudioReceiver*>(mDestinationCables[i]->GetTarget());
         if (target == nullptr)
         {
            RemovePatchCableSource(mDestinationCables[i]);
            cableSourceCount--;
         }
         else
            break;
      }
      mDestinationCables.resize(cableSourceCount);
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

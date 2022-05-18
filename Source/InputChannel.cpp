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
//  InputChannel.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/12.
//
//

#include "InputChannel.h"
#include "ModularSynth.h"
#include "Profiler.h"

InputChannel::InputChannel()
: IAudioProcessor(gBufferSize)
{
}

InputChannel::~InputChannel()
{
}

void InputChannel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChannelSelector = new DropdownList(this, "ch", 3, 3, &mChannelSelectionIndex);

   for (int i = 0; i < TheSynth->GetNumInputChannels(); ++i)
      mChannelSelector->AddLabel(ofToString(i + 1), i);
   mStereoSelectionOffset = mChannelSelector->GetNumValues(); //after this, the stereo pairs
   for (int i = 0; i < TheSynth->GetNumInputChannels() - 1; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1) + "&" + ofToString(i + 2), mChannelSelector->GetNumValues());
   mChannelSelector->DrawLabel(true);
   mChannelSelector->SetWidth(43);
}

void InputChannel::Process(double time)
{
   PROFILER(InputChannel);

   if (!mEnabled)
      return;

   int channelSelectionIndex = mChannelSelectionIndex;

   int numChannels = 1;
   if (mChannelSelectionIndex >= mStereoSelectionOffset)
      numChannels = 2;

   SyncBuffers(numChannels);

   IAudioReceiver* target = GetTarget();

   if (mChannelSelectionIndex < mStereoSelectionOffset) //mono
   {
      float* buffer = gZeroBuffer;
      int channel = mChannelSelectionIndex;
      if (channel >= 0 && channel < TheSynth->GetNumInputChannels())
         buffer = TheSynth->GetInputBuffer(channel);

      if (target)
         Add(target->GetBuffer()->GetChannel(0), buffer, gBufferSize);

      GetVizBuffer()->WriteChunk(buffer, gBufferSize, 0);
   }
   else //stereo
   {
      float* buffer1 = gZeroBuffer;
      float* buffer2 = gZeroBuffer;

      int channel1 = channelSelectionIndex - mStereoSelectionOffset;
      if (channel1 >= 0 && channel1 < TheSynth->GetNumInputChannels())
         buffer1 = TheSynth->GetInputBuffer(channel1);
      int channel2 = channel1 + 1;
      if (channel2 >= 0 && channel2 < TheSynth->GetNumInputChannels())
         buffer2 = TheSynth->GetInputBuffer(channel2);

      if (target)
      {
         Add(target->GetBuffer()->GetChannel(0), buffer1, gBufferSize);
         Add(target->GetBuffer()->GetChannel(1), buffer2, gBufferSize);
      }

      GetVizBuffer()->WriteChunk(buffer1, gBufferSize, 0);
      GetVizBuffer()->WriteChunk(buffer2, gBufferSize, 1);
   }
}

void InputChannel::DrawModule()
{
   mChannelSelector->Draw();

   if (gHoveredUIControl == mChannelSelector && TheSynth->GetNumInputChannels() == 0)
      TheSynth->SetNextDrawTooltip("selected input device has zero channels. choose a new audio_input_device in 'settings'.");
}

void InputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void InputChannel::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

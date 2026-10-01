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
   mChannelSelector->DrawLabel(true);
   mChannelSelector->SetWidth(43);
   RefreshChannels();
}

void InputChannel::RefreshChannels()
{
   if (mChannelSelector == nullptr)
      return;

   int count = TheSynth->GetNumInputChannels();
   mChannelSelector->Clear();
   for (int i = 0; i < count; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1), i);
   mStereoSelectionOffset = count;
   for (int i = 0; i < count - 1; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1) + "&" + ofToString(i + 2), count + i);

   mChannelSelectionIndex = mSelectedChannel + (mSelectedStereo ? mStereoSelectionOffset : 0);
   mUnavailableSelectionIndex = -1;
   if (mSelectedChannel + (mSelectedStereo ? 1 : 0) >= count)
   {
      mUnavailableSelectionIndex = count * 2 + mSelectedChannel + 1;
      mChannelSelectionIndex = mUnavailableSelectionIndex;
      std::string label = ofToString(mSelectedChannel + 1);
      if (mSelectedStereo)
         label += "&" + ofToString(mSelectedChannel + 2);
      mChannelSelector->AddLabel(label + " (unavailable)", mChannelSelectionIndex);
   }
   if (mModuleSaveData.HasProperty("channels"))
   {
      mModuleSaveData.SetEnumMapFromList("channels", mChannelSelector);
      mModuleSaveData.SetEnum("channels", mChannelSelectionIndex);
   }
}

void InputChannel::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mChannelSelector)
   {
      if (mChannelSelectionIndex != mUnavailableSelectionIndex)
      {
         mSelectedStereo = mStereoSelectionOffset > 0 && mChannelSelectionIndex >= mStereoSelectionOffset;
         mSelectedChannel = mChannelSelectionIndex - (mSelectedStereo ? mStereoSelectionOffset : 0);
      }
      RefreshChannels();
   }
}

void InputChannel::Process(double time)
{
   PROFILER(InputChannel);

   if (!mEnabled)
      return;

   int numChannels = mSelectedStereo ? 2 : 1;

   SyncBuffers(numChannels);

   IAudioReceiver* target = GetTarget();

   if (!mSelectedStereo) //mono
   {
      float* buffer = gZeroBuffer;
      int channel = mSelectedChannel;
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

      int channel1 = mSelectedChannel;
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

void InputChannel::GetModuleDimensions(float& width, float& height)
{
   width = MAX(64, (mChannelSelector ? (mChannelSelector->GetRect(true).getMaxX() + 3) : 0));
   height = 20;
}

void InputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadEnum<int>("channels", moduleInfo, 0, mChannelSelector);
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
   if (!moduleInfo["channel_index"].isNull() && !moduleInfo["stereo"].isNull())
   {
      mSelectedChannel = MAX(0, moduleInfo["channel_index"].asInt());
      mSelectedStereo = moduleInfo["stereo"].asBool();
      RefreshChannels();
   }
}

void InputChannel::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["channel_index"] = mSelectedChannel;
   moduleInfo["stereo"] = mSelectedStereo;
}

void InputChannel::SetUpFromSaveData()
{
   mChannelSelectionIndex = mModuleSaveData.GetEnum<int>("channels");
   mSelectedStereo = mStereoSelectionOffset > 0 && mChannelSelectionIndex >= mStereoSelectionOffset;
   mSelectedChannel = MAX(0, mChannelSelectionIndex - (mSelectedStereo ? mStereoSelectionOffset : 0));
   RefreshChannels();
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

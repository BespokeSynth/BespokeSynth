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
//  OutputChannel.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/17/12.
//
//

#include "OutputChannel.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

OutputChannel::OutputChannel()
: IAudioProcessor(gBufferSize)
{
}

OutputChannel::~OutputChannel()
{
}

void OutputChannel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChannelSelector = new DropdownList(this, "ch", 3, 3, &mChannelSelectionIndex);
   mChannelSelector->DrawLabel(true);
   mChannelSelector->SetWidth(43);
   RefreshChannels();

   GetPatchCableSource()->SetEnabled(false);
}

void OutputChannel::RefreshChannels()
{
   if (mChannelSelector == nullptr)
      return;

   int count = TheSynth->GetNumOutputChannels();
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

void OutputChannel::DropdownUpdated(DropdownList* list, int oldVal, double time)
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

void OutputChannel::Process(double time)
{
   int numChannels = GetNumChannels();

   SyncBuffers(numChannels);

   if (numChannels == 1)
   {
      int channel = mSelectedChannel;
      auto getBufferGetChannel0 = GetBuffer()->GetChannel(0);
      if (channel >= 0 && channel < TheSynth->GetNumOutputChannels())
      {
         if (mLimit > std::numeric_limits<float>::epsilon())
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel)[i] += std::clamp(getBufferGetChannel0[i], -mLimit, mLimit);
         }
         else
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel)[i] += getBufferGetChannel0[i];
         }
      }
      GetVizBuffer()->WriteChunk(getBufferGetChannel0, gBufferSize, 0);

      mLevelMeterDisplay.Process(0, channel >= 0 && channel < TheSynth->GetNumOutputChannels() ? TheSynth->GetOutputBuffer(channel) : gZeroBuffer, gBufferSize);
   }
   else //stereo
   {
      int channel1 = mSelectedChannel;
      if (channel1 >= 0 && channel1 < TheSynth->GetNumOutputChannels())
      {
         auto getBufferGetChannel0 = GetBuffer()->GetChannel(0);
         if (mLimit > std::numeric_limits<float>::epsilon())
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel1)[i] += CLAMP(getBufferGetChannel0[i], -mLimit, mLimit);
         }
         else
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel1)[i] += getBufferGetChannel0[i];
         }
         GetVizBuffer()->WriteChunk(getBufferGetChannel0, gBufferSize, 0);
      }
      int channel2 = channel1 + 1;
      int inputChannel2 = (GetBuffer()->NumActiveChannels() >= 2) ? 1 : 0;
      if (channel2 >= 0 && channel2 < TheSynth->GetNumOutputChannels())
      {
         auto getBufferGetChannel2 = GetBuffer()->GetChannel(inputChannel2);
         if (mLimit > std::numeric_limits<float>::epsilon())
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel2)[i] += CLAMP(getBufferGetChannel2[i], -mLimit, mLimit);
         }
         else
         {
            for (int i = 0; i < gBufferSize; ++i)
               TheSynth->GetOutputBuffer(channel2)[i] += getBufferGetChannel2[i];
         }
         GetVizBuffer()->WriteChunk(getBufferGetChannel2, gBufferSize, 1);
      }

      mLevelMeterDisplay.Process(0, channel1 >= 0 && channel1 < TheSynth->GetNumOutputChannels() ? TheSynth->GetOutputBuffer(channel1) : gZeroBuffer, gBufferSize);
      mLevelMeterDisplay.Process(1, channel2 >= 0 && channel2 < TheSynth->GetNumOutputChannels() ? TheSynth->GetOutputBuffer(channel2) : gZeroBuffer, gBufferSize);
   }

   GetBuffer()->Reset();
}

void OutputChannel::DrawModule()
{
   mChannelSelector->Draw();

   if (GetNumChannels() == 1)
   {
      mLevelMeterDisplay.Draw(3, 20, 58, 8, GetNumChannels());
      mHeight = 30;
   }
   else
   {
      mLevelMeterDisplay.Draw(3, 20, 58, 18, GetNumChannels());
      mHeight = 40;
   }
}

void OutputChannel::GetModuleDimensions(float& width, float& height)
{
   width = MAX(64, (mChannelSelector ? (mChannelSelector->GetRect(true).getMaxX() + 3) : 0));
   height = mHeight;
}

void OutputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   if (!moduleInfo["channel"].isNull())
      mModuleSaveData.LoadInt("channel", moduleInfo, 0, 0, TheSynth->GetNumOutputChannels() - 1);
   mModuleSaveData.LoadEnum<int>("channels", moduleInfo, 0, mChannelSelector);
   mModuleSaveData.LoadFloat("limit", moduleInfo, 1, 0, 1000, K(isTextField));

   SetUpFromSaveData();
   if (!moduleInfo["channel_index"].isNull() && !moduleInfo["stereo"].isNull())
   {
      mSelectedChannel = MAX(0, moduleInfo["channel_index"].asInt());
      mSelectedStereo = moduleInfo["stereo"].asBool();
      RefreshChannels();
   }
}

void OutputChannel::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["channel_index"] = mSelectedChannel;
   moduleInfo["stereo"] = mSelectedStereo;
}

void OutputChannel::SetUpFromSaveData()
{
   if (mModuleSaveData.HasProperty("channel")) //old version
      mChannelSelectionIndex = mModuleSaveData.GetInt("channel") - 1;
   else
      mChannelSelectionIndex = mModuleSaveData.GetEnum<int>("channels");
   mSelectedStereo = mStereoSelectionOffset > 0 && mChannelSelectionIndex >= mStereoSelectionOffset;
   mSelectedChannel = MAX(0, mChannelSelectionIndex - (mSelectedStereo ? mStereoSelectionOffset : 0));
   RefreshChannels();
   mLimit = mModuleSaveData.GetFloat("limit");

   mLevelMeterDisplay.SetLimit(mLimit);
}

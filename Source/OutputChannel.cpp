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

   for (int i = 0; i < TheSynth->GetNumOutputChannels(); ++i)
      mChannelSelector->AddLabel(ofToString(i + 1), i);
   mStereoSelectionOffset = mChannelSelector->GetNumValues(); //after this, the stereo pairs
   for (int i = 0; i < TheSynth->GetNumOutputChannels() - 1; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1) + "&" + ofToString(i + 2), mChannelSelector->GetNumValues());
   mChannelSelector->DrawLabel(true);
   mChannelSelector->SetWidth(43);

   GetPatchCableSource()->SetEnabled(false);
}

void OutputChannel::Process(double time)
{
   int numChannels = GetNumChannels();

   SyncBuffers(numChannels);

   int channelSelectionIndex = mChannelSelectionIndex;
   if (numChannels == 1)
   {
      int channel = channelSelectionIndex;
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

      mLevelMeterDisplay.Process(0, TheSynth->GetOutputBuffer(channel), gBufferSize);
   }
   else //stereo
   {
      int channel1 = channelSelectionIndex - mStereoSelectionOffset;
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

      mLevelMeterDisplay.Process(0, TheSynth->GetOutputBuffer(channel1), gBufferSize);
      mLevelMeterDisplay.Process(1, TheSynth->GetOutputBuffer(channel2), gBufferSize);
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

void OutputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   if (!moduleInfo["channel"].isNull())
      mModuleSaveData.LoadInt("channel", moduleInfo, 0, 0, TheSynth->GetNumOutputChannels() - 1);
   mModuleSaveData.LoadEnum<int>("channels", moduleInfo, 0, mChannelSelector);
   mModuleSaveData.LoadFloat("limit", moduleInfo, 1, 0, 1000, K(isTextField));

   SetUpFromSaveData();
}

void OutputChannel::SetUpFromSaveData()
{
   if (mModuleSaveData.HasProperty("channel")) //old version
      mChannelSelectionIndex = mModuleSaveData.GetInt("channel") - 1;
   else
      mChannelSelectionIndex = mModuleSaveData.GetEnum<int>("channels");
   mLimit = mModuleSaveData.GetFloat("limit");

   mLevelMeterDisplay.SetLimit(mLimit);
}

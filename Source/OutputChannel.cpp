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
   for (size_t i = 0; i < mLevelMeters.size(); ++i)
   {
      mLevelMeters[i].mPeakTrackerSlow.SetDecayTime(3);
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(1);
   }
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
         for (int i = 0; i < gBufferSize; ++i)
            TheSynth->GetOutputBuffer(channel)[i] += std::clamp(/*GetBuffer()->GetChannel(0)*/ getBufferGetChannel0[i], -mLimit, mLimit);
      }
      GetVizBuffer()->WriteChunk(/*GetBuffer()->GetChannel(0)*/ getBufferGetChannel0, gBufferSize, 0);

      mLevelMeters[0].mPeakTracker.Process(TheSynth->GetOutputBuffer(channel), gBufferSize);
      mLevelMeters[0].mPeakTrackerSlow.Process(TheSynth->GetOutputBuffer(channel), gBufferSize);
   }
   else //stereo
   {
      int channel1 = channelSelectionIndex - mStereoSelectionOffset;
      if (channel1 >= 0 && channel1 < TheSynth->GetNumOutputChannels())
      {
         for (int i = 0; i < gBufferSize; ++i)
            TheSynth->GetOutputBuffer(channel1)[i] += CLAMP(GetBuffer()->GetChannel(0)[i], -mLimit, mLimit);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), gBufferSize, 0);
      }
      int channel2 = channel1 + 1;
      int inputChannel2 = (GetBuffer()->NumActiveChannels() >= 2) ? 1 : 0;
      if (channel2 >= 0 && channel2 < TheSynth->GetNumOutputChannels())
      {
         for (int i = 0; i < gBufferSize; ++i)
            TheSynth->GetOutputBuffer(channel2)[i] += CLAMP(GetBuffer()->GetChannel(inputChannel2)[i], -mLimit, mLimit);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(inputChannel2), gBufferSize, 1);
      }

      mLevelMeters[0].mPeakTracker.Process(TheSynth->GetOutputBuffer(channel1), gBufferSize);
      mLevelMeters[0].mPeakTrackerSlow.Process(TheSynth->GetOutputBuffer(channel1), gBufferSize);
      mLevelMeters[1].mPeakTracker.Process(TheSynth->GetOutputBuffer(channel2), gBufferSize);
      mLevelMeters[1].mPeakTrackerSlow.Process(TheSynth->GetOutputBuffer(channel2), gBufferSize);
   }

   GetBuffer()->Reset();
}

void OutputChannel::DrawModule()
{
   mChannelSelector->Draw();

   if (GetNumChannels() == 1)
      mHeight = 30;
   else
      mHeight = 40;

   for (int i = 0; i < GetNumChannels(); ++i)
   {
      const int kNumSegments = 20;
      const int kPaddingOutside = 3;
      const int kPaddingBetween = 1;
      const int kBarHeight = 8;
      const float kSegmentWidth = (mWidth - kPaddingOutside * 2) / kNumSegments;
      for (int j = 0; j < kNumSegments; ++j)
      {
         ofPushStyle();
         ofFill();
         float level = mLevelMeters[i].mPeakTracker.GetPeak() / mLimit;
         float slowLevel = mLevelMeters[i].mPeakTrackerSlow.GetPeak() / mLimit;
         ofColor color(0, 255, 0);
         if (j > kNumSegments - 3)
            color.set(255, 0, 0);
         else if (j > kNumSegments - 6)
            color.set(255, 255, 0);

         if (slowLevel > 0 && ofClamp(int(slowLevel * kNumSegments), 0, kNumSegments - 1) == j)
            ofSetColor(color);
         else if (level > 0 && level >= j / (float)kNumSegments)
            ofSetColor(color * .9f);
         else
            ofSetColor(color * .5f);
         ofRect(kPaddingOutside + kSegmentWidth * j, 20 + i * (kBarHeight + 2), kSegmentWidth - kPaddingBetween, kBarHeight, 0);
         ofPopStyle();
      }

      if (mLevelMeters[i].mPeakTrackerSlow.GetLastHitLimitTime() > gTime - 1000)
      {
         ofPushStyle();
         ofSetColor(ofColor::red);
         DrawTextBold("clipped", kPaddingOutside + 10, 20 + i * (kBarHeight + 2) + 8, 12.0f);
         ofPopStyle();
      }
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

   for (size_t i = 0; i < mLevelMeters.size(); ++i)
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(mLimit);
}

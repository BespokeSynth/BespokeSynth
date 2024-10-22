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
//  MidiClockIn.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/22.
//
//

#include "MidiClockIn.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "Transport.h"

MidiClockIn::MidiClockIn()
: mDevice(this)
{
   mTempoHistory.fill(0);
}

MidiClockIn::~MidiClockIn()
{
}

void MidiClockIn::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 3, 120);
   DROPDOWN(mDeviceList, "device", &mDeviceIndex, 120);
   DROPDOWN(mTempoRoundModeList, "rounding", ((int*)&mTempoRoundMode), 50);
   FLOATSLIDER(mStartOffsetMsSlider, "start offset ms", &mStartOffsetMs, -300, 300);
   INTSLIDER(mSmoothAmountSlider, "smoothing", &mSmoothAmount, 1, (int)mTempoHistory.size());
   ENDUIBLOCK(mWidth, mHeight);

   mTempoRoundModeList->DrawLabel(true);
   mTempoRoundModeList->AddLabel("none", (int)TempoRoundMode::kNone);
   mTempoRoundModeList->AddLabel("1", (int)TempoRoundMode::kWhole);
   mTempoRoundModeList->AddLabel(".5", (int)TempoRoundMode::kHalf);
   mTempoRoundModeList->AddLabel(".25", (int)TempoRoundMode::kQuarter);
   mTempoRoundModeList->AddLabel(".1", (int)TempoRoundMode::kTenth);

   mHeight += 20;
}

void MidiClockIn::Init()
{
   IDrawableModule::Init();

   InitDevice();
}

void MidiClockIn::InitDevice()
{
   BuildDeviceList();

   const std::vector<std::string>& devices = mDevice.GetPortList(true);
   for (int i = 0; i < devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(), mDevice.Name()) == 0)
         mDeviceIndex = i;
   }
}

float MidiClockIn::GetRoundedTempo()
{
   float avgTempo = 0;
   int temposToCount = std::min((int)mTempoHistory.size(), mSmoothAmount);

   for (int i = 0; i < temposToCount; ++i)
      avgTempo += mTempoHistory[(mTempoIdx - 1 - i + (int)mTempoHistory.size()) % (int)mTempoHistory.size()];
   avgTempo /= temposToCount;

   switch (mTempoRoundMode)
   {
      case TempoRoundMode::kNone:
         return avgTempo;
      case TempoRoundMode::kWhole:
         return round(avgTempo);
      case TempoRoundMode::kHalf:
         return round(avgTempo * 2) / 2;
      case TempoRoundMode::kQuarter:
         return round(avgTempo * 4) / 4;
      case TempoRoundMode::kTenth:
         return round(avgTempo * 10) / 10;
   }

   return avgTempo;
}

void MidiClockIn::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDeviceList->Draw();
   mTempoRoundModeList->Draw();
   mStartOffsetMsSlider->Draw();
   mSmoothAmountSlider->Draw();

   DrawTextNormal("tempo: " + ofToString(GetRoundedTempo()), 4, mHeight - 5);
}

void MidiClockIn::DrawModuleUnclipped()
{
   if (mDrawDebug)
      DrawTextNormal(mDebugDisplayText, 0, mHeight + 20);
}

void MidiClockIn::BuildDeviceList()
{
   mDeviceList->Clear();
   const std::vector<std::string>& devices = mDevice.GetPortList(true);
   for (int i = 0; i < devices.size(); ++i)
      mDeviceList->AddLabel(devices[i].c_str(), i);
}

void MidiClockIn::OnMidi(const juce::MidiMessage& message)
{
   const int kDebugMaxLineCount = 40;

   if (message.isMidiClock() || message.isSongPositionPointer())
   {
      const int kMinRequiredPulseCount = 48;
      double time = message.getTimeStamp();
      if (mDrawDebug)
         AddDebugLine("midi clock " + ofToString(time, 3), kDebugMaxLineCount);

      if (mReceivedPulseCount == 0)
      {
         double currentTempoDeltaSeconds = 1.0 / (TheTransport->GetTempo() / 60 * 24);
         mDelayLockedLoop.reset(time, currentTempoDeltaSeconds, 1);
         mDelayLockedLoop.setParams(gBufferSize, gSampleRate);
      }
      else
      {
         mDelayLockedLoop.update(time);
      }

      if (mReceivedPulseCount >= kMinRequiredPulseCount && time - mLastTimestamp > .001f)
      {
         double deltaSeconds = mDelayLockedLoop.timeDiff();
         double pulsesPerSecond = 1 / deltaSeconds;
         double beatsPerSecond = pulsesPerSecond / 24;
         double instantTempo = beatsPerSecond * 60;

         if (instantTempo > 20 && instantTempo < 999)
         {
            if (mTempoIdx == -1)
               mTempoHistory.fill(instantTempo);
            else
               mTempoHistory[mTempoIdx] = instantTempo;
            mTempoIdx = (mTempoIdx + 1) % mTempoHistory.size();

            if (mEnabled)
               TheTransport->SetTempo(GetRoundedTempo());
         }

         if (mDrawDebug)
            AddDebugLine("   deltaSeconds=" + ofToString(deltaSeconds, 6) + " instantTempo=" + ofToString(instantTempo, 2) + " rounded tempo:" + ofToString(GetRoundedTempo(), 1), kDebugMaxLineCount);

         mLastTimestamp = time;
      }
      ++mReceivedPulseCount;
   }
   if (message.isMidiStart())
   {
      if (mDrawDebug)
         AddDebugLine("midi start", kDebugMaxLineCount);
      if (mObeyClockStartStop)
      {
         mTempoIdx = -1;
         mReceivedPulseCount = 0;
         if (TheSynth->IsAudioPaused())
         {
            TheSynth->SetAudioPaused(false);
            TheTransport->Reset();
         }
      }
   }
   if (message.isMidiStop())
   {
      if (mDrawDebug)
         AddDebugLine("midi stop", kDebugMaxLineCount);
      if (mObeyClockStartStop)
         TheSynth->SetAudioPaused(true);
   }
   if (message.isMidiContinue())
   {
      if (mDrawDebug)
         AddDebugLine("midi continue", kDebugMaxLineCount);

      if (mObeyClockStartStop)
      {
         mTempoIdx = -1;
         mReceivedPulseCount = 0;
         TheSynth->SetAudioPaused(false);
      }
   }
   if (message.isSongPositionPointer())
   {
      if (mDrawDebug)
         AddDebugLine("midi position pointer " + ofToString(message.getSongPositionPointerMidiBeat()), kDebugMaxLineCount);

      if (mEnabled)
         TheTransport->SetMeasureTime(message.getSongPositionPointerMidiBeat() / TheTransport->GetTimeSigTop() + mStartOffsetMs / TheTransport->MsPerBar());
   }
   if (message.isQuarterFrame())
   {
      if (mDrawDebug)
         AddDebugLine("midi quarter frame " + ofToString(message.getQuarterFrameValue()) + " " + ofToString(message.getQuarterFrameSequenceNumber()), kDebugMaxLineCount);
   }
}

void MidiClockIn::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mDeviceList)
   {
      mDevice.ConnectInput(mDeviceIndex);
   }
}

void MidiClockIn::DropdownClicked(DropdownList* list)
{
   BuildDeviceList();
}

void MidiClockIn::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("obey_clock_start_stop", moduleInfo, true);
   SetUpFromSaveData();
}

void MidiClockIn::SetUpFromSaveData()
{
   mObeyClockStartStop = mModuleSaveData.GetBool("obey_clock_start_stop");
}

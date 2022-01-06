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
#include "IAudioSource.h"
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
   
   UIBLOCK(3,3,120);
   DROPDOWN(mDeviceList, "device", &mDeviceIndex, 120);
   DROPDOWN(mTempoRoundModeList, "rounding", ((int*)&mTempoRoundMode), 40);
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
   for (int i=0; i<devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(),mDevice.Name()) == 0)
         mDeviceIndex = i;
   }
}

float MidiClockIn::GetRoundedTempo()
{
   float avgTempo = 0;
   int temposToCount = std::min((int)mTempoHistory.size(), mSmoothAmount);

   for (int i = 0; i < temposToCount; ++i)
      avgTempo += mTempoHistory[(mTempoIdx - 1 - i+ (int)mTempoHistory.size()) % (int)mTempoHistory.size()];
   avgTempo /= temposToCount;

   switch (mTempoRoundMode)
   {
      case TempoRoundMode::kNone:
         return avgTempo;
      case TempoRoundMode::kWhole:
         return round(avgTempo);
      case TempoRoundMode::kHalf:
         return round(avgTempo*2)/2;
      case TempoRoundMode::kQuarter:
         return round(avgTempo*4)/4;
      case TempoRoundMode::kTenth:
         return round(avgTempo*10)/10;
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

void MidiClockIn::BuildDeviceList()
{
   mDeviceList->Clear();
   const std::vector<std::string>& devices = mDevice.GetPortList(true);
   for (int i=0; i<devices.size(); ++i)
      mDeviceList->AddLabel(devices[i].c_str(), i);
}

void MidiClockIn::OnMidi(const juce::MidiMessage& message)
{
   if (message.isMidiClock())
   {
      if (mLastTimestamp > 0)
      {
         double deltaSeconds = (message.getTimeStamp() - mLastTimestamp);
         double pulsesPerSecond = 1 / deltaSeconds;
         double beatsPerSecond = pulsesPerSecond / 24;
         double instantTempo = beatsPerSecond * 60;
         
         if (mTempoIdx == -1)
            mTempoHistory.fill(instantTempo);
         else
            mTempoHistory[mTempoIdx] = instantTempo;
         mTempoIdx = (mTempoIdx + 1) % mTempoHistory.size();
         
         if (mEnabled)
            TheTransport->SetTempo(GetRoundedTempo());
      }
      mLastTimestamp = message.getTimeStamp();
   }
   if (message.isMidiStart())
   {
      ofLog() << "midi start";
   }
   if (message.isMidiStop())
   {
      ofLog() << "midi stop";
   }
   if (message.isMidiContinue())
   {
      ofLog() << "midi continue";
   }
   if (message.isSongPositionPointer())
   {
      ofLog() << "midi position pointer " << ofToString(message.getSongPositionPointerMidiBeat());
      
      if (mEnabled)
         TheTransport->SetMeasureTime(message.getSongPositionPointerMidiBeat() / TheTransport->GetTimeSigTop() + mStartOffsetMs / TheTransport->MsPerBar());
   }
   if (message.isQuarterFrame())
   {
      //ofLog() << "midi quarter frame " << ofToString(message.getQuarterFrameValue()) << " " << ofToString(message.getQuarterFrameSequenceNumber());
   }
}

void MidiClockIn::DropdownUpdated(DropdownList* list, int oldVal)
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
   SetUpFromSaveData();
}

void MidiClockIn::SetUpFromSaveData()
{
}

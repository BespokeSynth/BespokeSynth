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
//  MidiClockOut.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/22.
//
//

#include "MidiClockOut.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

MidiClockOut::MidiClockOut()
: mDevice(nullptr)
{
}

MidiClockOut::~MidiClockOut()
{
   TheTransport->RemoveAudioPoller(this);
}

void MidiClockOut::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   DROPDOWN(mDeviceList, "device", &mDeviceIndex, 120);
   ENDUIBLOCK(mWidth, mHeight);
}

void MidiClockOut::Init()
{
   IDrawableModule::Init();
   
   InitDevice();

   TheTransport->AddAudioPoller(this);
}

void MidiClockOut::InitDevice()
{
   BuildDeviceList();
   
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(),mDevice.Name()) == 0)
         mDeviceIndex = i;
   }
}

void MidiClockOut::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mDeviceList->Draw();
}

void MidiClockOut::BuildDeviceList()
{
   mDeviceList->Clear();
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
      mDeviceList->AddLabel(devices[i].c_str(), i);
}

void MidiClockOut::OnTransportAdvanced(float amount)
{
   if (mEnabled)
   {
      double oldPulse = TheTransport->GetMeasureTime(gTime) * TheTransport->GetTimeSigTop() * 24;
      double newPulse = TheTransport->GetMeasureTime(gTime+gBufferSizeMs) * TheTransport->GetTimeSigTop() * 24;
      int pulses = int(floor(newPulse) - floor(oldPulse));
      double distToFirstPulse = 1 - fmod(oldPulse, 1);
      double pulseMs = TheTransport->GetDuration(kInterval_4n) / 24;
      for (int i=0; i<pulses; ++i)
         mDevice.SendMessage(gTime + (distToFirstPulse + i) * pulseMs, juce::MidiMessage::midiClock());
   }
}

void MidiClockOut::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mDeviceList)
   {
      mDevice.ConnectOutput(mDeviceIndex);
   }
}

void MidiClockOut::DropdownClicked(DropdownList* list)
{
   BuildDeviceList();
}

void MidiClockOut::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void MidiClockOut::SetUpFromSaveData()
{
}

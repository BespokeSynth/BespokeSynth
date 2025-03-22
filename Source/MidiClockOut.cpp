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
#include "ModularSynth.h"
#include "UIControlMacros.h"

MidiClockOut::MidiClockOut()
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
   BUTTON(mStartButton, "send start");
   DROPDOWN(mMultiplierSelector, "multiplier", ((int*)&mMultiplier), 70);
   ENDUIBLOCK(mWidth, mHeight);

   mMultiplierSelector->AddLabel("quarter", (int)ClockMultiplier::Quarter);
   mMultiplierSelector->AddLabel("half", (int)ClockMultiplier::Half);
   mMultiplierSelector->AddLabel("one", (int)ClockMultiplier::One);
   mMultiplierSelector->AddLabel("two", (int)ClockMultiplier::Two);
   mMultiplierSelector->AddLabel("four", (int)ClockMultiplier::Four);
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
   for (int i = 0; i < devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(), mDevice.Name()) == 0)
         mDeviceIndex = i;
   }
}

void MidiClockOut::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDeviceList->Draw();
   mStartButton->Draw();
   mMultiplierSelector->Draw();
}

void MidiClockOut::BuildDeviceList()
{
   mDeviceList->Clear();
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
      mDeviceList->AddLabel(devices[i].c_str(), i);
}

void MidiClockOut::OnTransportAdvanced(float amount)
{
   if (mEnabled)
   {
      int pulsesPerBeat = 24;
      switch (mMultiplier)
      {
         case ClockMultiplier::Quarter: pulsesPerBeat /= 4; break;
         case ClockMultiplier::Half: pulsesPerBeat /= 2; break;
         case ClockMultiplier::One: break;
         case ClockMultiplier::Two: pulsesPerBeat *= 2; break;
         case ClockMultiplier::Four: pulsesPerBeat *= 4; break;
      }
      int pulsesPerMeasure = TheTransport->GetTimeSigTop() * pulsesPerBeat;

      double oldPulse = TheTransport->GetMeasureTime(gTime) * pulsesPerMeasure;
      double newPulse = TheTransport->GetMeasureTime(NextBufferTime(false)) * pulsesPerMeasure;
      int pulses = int(floor(newPulse) - floor(oldPulse));
      double distToFirstPulse = 1 - fmod(oldPulse, 1);
      double pulseMs = TheTransport->GetDuration(kInterval_4n) / pulsesPerBeat;
      for (int i = 0; i < pulses; ++i)
      {
         if (mClockStartQueued && (int(oldPulse) + i) % pulsesPerMeasure == 0)
         {
            mDevice.SendMessage(gTime + (distToFirstPulse + i) * pulseMs, juce::MidiMessage::midiStart());
            mClockStartQueued = false;
         }
         else
         {
            mDevice.SendMessage(gTime + (distToFirstPulse + i) * pulseMs, juce::MidiMessage::midiClock());
         }
      }
   }
}

void MidiClockOut::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mDeviceList)
   {
      mDevice.ConnectOutput(mDeviceIndex);
   }
}

void MidiClockOut::ButtonClicked(ClickButton* button, double time)
{
   if (button == mStartButton)
      mClockStartQueued = true;
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

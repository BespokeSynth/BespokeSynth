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
//  Monome.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/14.
//
//

#pragma once

#include "MidiDevice.h"
#include "INonstandardController.h"

#include "juce_osc/juce_osc.h"

#define HOST "127.0.0.1"
#define SERIAL_OSC_PORT 12002

#define NUM_MONOME_BUTTONS 128

class DropdownList;

class Monome : public INonstandardController,
               private juce::OSCReceiver,
               private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
   Monome(MidiDeviceListener* listener);
   ~Monome();

   void Poll() override;
   bool SetUpOsc();
   void ListMonomes();
   void SetLight(int x, int y, float value);
   void SetLightFlicker(int x, int y, float flickerMs);
   std::string GetControlTooltip(MidiMessageType type, int control) override;
   void SetLayoutData(ofxJSONElement& layout) override;
   void ConnectToDevice(std::string deviceDesc);
   void UpdateDeviceList(DropdownList* list);

   void oscMessageReceived(const juce::OSCMessage& msg) override;

   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) override;

   bool IsInputConnected() override { return mHasMonome; }
   bool Reconnect() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   void SetLightInternal(int x, int y, float value);
   Vec2i Rotate(int x, int y, int rotations);

   static int sNextMonomeReceivePort;

   juce::OSCSender mToSerialOsc;
   juce::OSCSender mToMonome;
   int mMonomeReceivePort{ -1 };
   bool mIsOscSetUp{ false };
   bool mHasMonome{ false };
   bool mLightsInitialized{ false };
   int mMaxColumns{ 16 };
   int mGridRotation{ 0 };
   juce::String mPrefix{ "monome" };
   bool mJustRequestedDeviceList{ false };
   std::string mPendingDeviceDesc;

   struct MonomeDevice
   {
      void CopyFrom(MonomeDevice& other)
      {
         id = other.id;
         product = other.product;
         port = other.port;
      }
      std::string id;
      std::string product;
      int port{ 0 };
      std::string GetDescription() { return id + " " + product; }
   };

   std::vector<MonomeDevice> mConnectedDeviceList;

   MidiDeviceListener* mListener{ nullptr };
   DropdownList* mListForMidiController{ nullptr };
   MonomeDevice mLastConnectedDeviceInfo;

   struct LightInfo
   {
      float mValue{ 0 };
      double mLastUpdatedTime{ 0 };
      double mLastSentTime{ 0 };
   };

   std::vector<LightInfo> mLights;
};

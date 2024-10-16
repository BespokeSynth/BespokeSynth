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
//  OscController.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/14.
//
//

#pragma once

#include "MidiDevice.h"
#include "INonstandardController.h"

#include "juce_osc/juce_osc.h"

struct OscMap
{
   int mControl{ 0 };
   std::string mAddress;
   bool mIsFloat{ false };
   float mFloatValue{ 0 };
   int mIntValue{ 0 };
   double mLastChangedTime{ -9999 }; //@TODO(Noxy): Unused but is in savestates.
};

class OscController : public INonstandardController,
                      private juce::OSCReceiver,
                      private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
   OscController(MidiDeviceListener* listener, std::string outAddress, int outPort, int inPort);
   ~OscController();

   void Connect();
   void oscMessageReceived(const juce::OSCMessage& msg) override;
   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) override;
   int AddControl(std::string address, bool isFloat);

   bool IsInputConnected() override { return mConnected; }
   bool Reconnect() override
   {
      Connect();
      return mConnected;
   }
   bool SetInPort(int port);
   std::string GetControlTooltip(MidiMessageType type, int control) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   MidiDeviceListener* mListener{ nullptr };

   int FindControl(std::string address);
   void ConnectOutput();

   std::string mOutAddress;
   int mOutPort{ 0 };
   int mInPort{ 0 };
   juce::OSCSender mOscOut;
   bool mConnected{ false };
   bool mOutputConnected{ false };

   std::vector<OscMap> mOscMap;
};

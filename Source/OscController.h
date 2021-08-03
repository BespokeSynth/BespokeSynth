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

#ifndef __Bespoke__OscController__
#define __Bespoke__OscController__

#include <iostream>
#include "MidiDevice.h"
#include "INonstandardController.h"
#include "ofxJSONElement.h"

struct OscMap
{
   int mControl;
   string mAddress;
   bool mIsFloat;
   float mFloatValue;
   int mIntValue;
   double mLastChangedTime;
};

class OscController : public INonstandardController,
                      private OSCReceiver,
                      private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:
   OscController(MidiDeviceListener* listener, string outAddress, int outPort, int inPort);
   ~OscController();
   
   void Connect();
   void oscMessageReceived(const OSCMessage& msg) override;
   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) override;
   int AddControl(string address, bool isFloat);
   
   bool IsInputConnected() override { return mConnected; }
   bool Reconnect() override { Connect(); return mConnected; }
   bool SetInPort(int port);
   string GetControlTooltip(MidiMessageType type, int control) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   MidiDeviceListener* mListener;

   int FindControl(string address);
   void ConnectOutput();
   
   string mOutAddress;
   int mOutPort;
   int mInPort;
   OSCSender mOscOut;
   bool mConnected;
   bool mOutputConnected;
   
   vector<OscMap> mOscMap;
};

#endif /* defined(__Bespoke__OscController__) */

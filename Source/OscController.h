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
   
   bool IsInputConnected() override { return mConnected; }
   bool Reconnect() override { Connect(); return mConnected; }
   bool SetInPort(int port);
   string GetControlTooltip(MidiMessageType type, int control) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   MidiDeviceListener* mListener;

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

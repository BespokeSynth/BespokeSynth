//
//  OscController.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/14.
//
//

#include "OscController.h"
#include "SynthGlobals.h"

OscController::OscController(MidiDeviceListener* listener, string outAddress, int outPort, int inPort)
: mListener(listener)
, mConnected(false)
, mOutAddress(outAddress)
, mOutPort(outPort)
, mInPort(inPort)
{
   Connect();
}

OscController::~OscController()
{
   OSCReceiver::disconnect();
}

void OscController::Connect()
{
   try
   {
      bool connected = OSCReceiver::connect(mInPort);
      assert(connected);
      
      OSCReceiver::addListener(this);
      
      connected = mOscOut.connect( mOutAddress, mOutPort );
      assert(connected);
      
      mConnected = true;
   }
   catch (exception e)
   {
   }
}

void OscController::SendValue(int page, int control, float value, bool forceNoteOn /*= false*/, int channel /*= -1*/)
{
   if (!mConnected)
      return;
   
   for (int i=0; i<mOscMap.size(); ++i)
   {
      if (control == mOscMap[i].mControl && mOscMap[i].mLastChangedTime + 50 < gTime)
      {
         mOscMap[i].mValue = value;
         
         OSCMessage msg(mOscMap[i].mAddress.c_str());
         
         map<int, float> values;
         for (int j=0; j<mOscMap.size(); ++j)
         {
            if (mOscMap[i].mAddress == mOscMap[j].mAddress)
               values[mOscMap[j].mIndex] = mOscMap[j].mValue;
         }
         
         for (int j=0; j<values.size(); ++j)
            msg.addFloat32(values[j]);
         
         mOscOut.send(msg);
      }
   }
}

void OscController::oscMessageReceived(const OSCMessage& msg)
{
   for (int j=0; j < msg.size(); ++j)
   {
      for (int i=0; i<mOscMap.size(); ++i)
      {
         if (mOscMap[i].mIndex == j && msg.getAddressPattern().toString().toStdString() == mOscMap[i].mAddress)
         {
            mOscMap[i].mLastChangedTime = gTime;
            mOscMap[i].mValue = msg[j].getFloat32();
            MidiControl control;
            control.mControl = mOscMap[i].mControl;
            control.mValue = mOscMap[i].mValue * 127;
            control.mDeviceName = "osccontroller";
            mListener->OnMidiControl(control);
         }
      }
   }
}

void OscController::LoadInfo(const ofxJSONElement& moduleInfo)
{
   const ofxJSONElement& connections = moduleInfo["connections"];
   
   for (int i=0; i<connections.size(); ++i)
   {
      OscMap oscMap;
      oscMap.mControl = connections[i]["control"].asInt();
      oscMap.mAddress = connections[i]["oscaddress"].asString();
      oscMap.mIndex = connections[i]["oscidx"].asInt();
      oscMap.mValue = 0;
      oscMap.mLastChangedTime = -9999;
      mOscMap.push_back(oscMap);
   }
}


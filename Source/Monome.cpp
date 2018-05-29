//
//  Monome.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/14.
//
//

#include "Monome.h"
#include "Profiler.h"

Monome::Monome(MidiDeviceListener* listener)
: mHasMonome(false)
, mMaxColumns(8)
, mListener(listener)
{
   bool connected = OSCReceiver::connect(MONOME_RECEIVE_PORT);
   assert(connected);
   
   OSCReceiver::addListener(this);
   
	connected = mToSerialOsc.connect( HOST, SERIAL_OSC_PORT );
   assert(connected);
   
   Connect();
}

Monome::~Monome()
{
   OSCReceiver::disconnect();
}

void Monome::Connect()
{
   OSCMessage listMsg("/serialosc/list");
   listMsg.addString("localhost");
   listMsg.addInt32(MONOME_RECEIVE_PORT);
   bool written = mToSerialOsc.send(listMsg);
   assert(written);
}

void Monome::SetLightInternal(int x, int y, bool on)
{
   if (!mHasMonome)
      return;
   
   OSCMessage lightMsg("/monome/grid/led/set");
   lightMsg.addInt32(x);
   lightMsg.addInt32(y);
   lightMsg.addInt32(on ? 1 : 0);
   bool written = mToMonome.send(lightMsg);
   assert(written);
}

void Monome::SetLight(int x, int y, bool on)
{
   SetLightInternal(x, y, on);
}

void Monome::SetLightFlicker(int x, int y, float intensity)
{
   if (intensity == 0)
   {
      SetLight(x,y,false);
      return;
   }
   if (intensity == 1)
   {
      SetLight(x,y,true);
      return;
   }
}

void Monome::oscMessageReceived(const OSCMessage& msg)
{
   String label = msg.getAddressPattern().toString();
   
   if (label == "/serialosc/device")
   {
      ofLog() << "Monome connected: " << msg[0].getString().toRawUTF8() << " " << msg[1].getString().toRawUTF8();
      
      //TODO(Ryan) set rows and columns here based on monome size
      
      mToMonome.connect(HOST, msg[2].getInt32());
      mHasMonome = true;
      
      OSCMessage setPortMsg("/sys/port");
      setPortMsg.addInt32(MONOME_RECEIVE_PORT);
      bool written = mToMonome.send(setPortMsg);
      assert(written);
      
      OSCMessage setHostMsg("/sys/host");
      setHostMsg.addString(HOST);
      written = mToMonome.send(setHostMsg);
      assert(written);
      
      OSCMessage setPrefixMsg("/sys/prefix");
      setPrefixMsg.addString("monome");
      written = mToMonome.send(setPrefixMsg);
      assert(written);
      
      /*OSCMessage setTiltMsg("/monome/tilt/set");
      setTiltMsg.addInt32(0);
      setTiltMsg.addInt32(1);
      mToMonome.send(setTiltMsg);*/
   }
   else if (label == "/monome/grid/key")
   {
      int row = msg[0].getInt32();
      int col = msg[1].getInt32();
      int val = msg[2].getInt32();
      
      MidiNote note;
      note.mPitch = row + col*mMaxColumns;
      note.mVelocity = val * 127;
      note.mChannel = 0;
      note.mDeviceName = "monome";
      mListener->OnMidiNote(note);
   }
   else if (label == "/monome/tilt")
   {
      /*MidiControl updown;
      updown.mControl = 1;
      updown.mValue = ofMap(msg[1].getInt32(), 88.0f, 164.0f, 0.0f, 1.0f, true);
      mListener->OnMidiControl(updown);
      
      MidiControl leftright;
      leftright.mControl = 0;
      leftright.mValue = 1-ofMap(msg[2].getInt32(), 88.0f, 164.0f, 0.0f, 1.0f, true);
      mListener->OnMidiControl(leftright);*/
   }
}

void Monome::SendValue(int page, int control, float value, bool forceNoteOn /*= false*/, int channel /*= -1*/)
{
   //SetLightFlicker(control%8,control/8,value);
   SetLight(control%mMaxColumns,control/mMaxColumns,value>0);
}

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
//  Monome.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/14.
//
//

#include "Monome.h"
#include "Profiler.h"

Monome::Monome(MidiDeviceListener* listener)
: mIsOscSetUp(false)
, mHasMonome(false)
, mMaxColumns(16)
, mGridRotation(0)
, mPrefix("monome")
, mListener(listener)
{
   Connect();
}

Monome::~Monome()
{
   OSCReceiver::disconnect();
}

bool Monome::SetUpOsc()
{
   assert(!mIsOscSetUp);
   
   bool connected = OSCReceiver::connect(MONOME_RECEIVE_PORT);
   if (!connected)
      return false;
   
   OSCReceiver::addListener(this);
   
   connected = mToSerialOsc.connect( HOST, SERIAL_OSC_PORT );
   if (!connected)
      return false;
   
   mIsOscSetUp = true;
   return true;
}

void Monome::Connect()
{
   if (mHasMonome)
      return;

   if (!mIsOscSetUp)
   {
      bool success = SetUpOsc();
      if (!success)
         return;
   }
   
   OSCMessage listMsg("/serialosc/list");
   listMsg.addString("localhost");
   listMsg.addInt32(MONOME_RECEIVE_PORT);
   bool written = mToSerialOsc.send(listMsg);
   assert(written);
}

void Monome::SetLightInternal(int x, int y, float value)
{
   if (!mHasMonome)
      return;
   
   OSCMessage lightMsg("/"+mPrefix+"/grid/led/level/set");
   Vec2i pos = Rotate(x, y, mGridRotation);
   lightMsg.addInt32(pos.x);
   lightMsg.addInt32(pos.y);
   lightMsg.addInt32(value*16);
   bool written = mToMonome.send(lightMsg);
   assert(written);
}

void Monome::SetLight(int x, int y, float value)
{
   SetLightInternal(x, y, value);
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

string Monome::GetControlTooltip(MidiMessageType type, int control)
{
   if (type == kMidiMessage_Note)
      return "(" + ofToString(control % mMaxColumns) + ", " + ofToString(control/mMaxColumns) + ")";
   return MidiController::GetDefaultTooltip(type, control);
}

void Monome::SetLayoutData(ofxJSONElement& layout)
{
   if (!layout["monome_rotation"].isNull())
      mGridRotation = layout["monome_rotation"].asInt();
   if (!layout["monome_prefix"].isNull())
      mPrefix = layout["monome_prefix"].asString();
}

void Monome::oscMessageReceived(const OSCMessage& msg)
{
   String label = msg.getAddressPattern().toString();
   
   if (label == "/serialosc/device")
   {
      ofLog() << "Monome connected: " << msg[0].getString().toRawUTF8() << " " << msg[1].getString().toRawUTF8();
      
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
      setPrefixMsg.addString(mPrefix);
      written = mToMonome.send(setPrefixMsg);
      assert(written);
      
      OSCMessage sysInfoMsg("/sys/info");
      written = mToMonome.send(sysInfoMsg);
      assert(written);
      
      /*OSCMessage setTiltMsg("/"+mPrefix+"/tilt/set");
      setTiltMsg.addInt32(0);
      setTiltMsg.addInt32(1);
      mToMonome.send(setTiltMsg);*/
   }
   else if (label == "/sys/size")
   {
      mMaxColumns = msg[0].getInt32();
   }
   else if (label == "/"+mPrefix+"/grid/key")
   {
      Vec2i pos = Rotate(msg[0].getInt32(), msg[1].getInt32(), -mGridRotation);
      int val = msg[2].getInt32();
      
      MidiNote note;
      note.mPitch = pos.x + pos.y*mMaxColumns;
      note.mVelocity = val * 127.0f;
      note.mChannel = 0;
      note.mDeviceName = mPrefix.toUTF8();
      mListener->OnMidiNote(note);
   }
   else if (label == "/"+mPrefix+"/tilt")
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
   SetLight(control%mMaxColumns,control/mMaxColumns,value);
}

Vec2i Monome::Rotate(int x, int y, int rotations)
{
   if (rotations < 0)
      rotations += 4;
   Vec2i ret(x, y);
   for (int i = 0; i < rotations; ++i)
   {
      x = ret.y;
      y = mMaxColumns - 1 - ret.x;
      ret.x = x;
      ret.y = y;
   }
   return ret;
}

namespace
{
   const int kSaveStateRev = 1;
}

void Monome::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
}

void Monome::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
}

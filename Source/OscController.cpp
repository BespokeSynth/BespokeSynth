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
, mOutputConnected(false)
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
      
      ConnectOutput();
      
      mConnected = true;
   }
   catch (exception e)
   {
   }
}

void OscController::ConnectOutput()
{
   if (mOutAddress != "" && mOutPort > 0)
   {
      mOutputConnected = mOscOut.connect(mOutAddress, mOutPort);
   }
}

bool OscController::SetInPort(int port)
{
   if (mInPort != port)
   {
      mInPort = port;
      OSCReceiver::disconnect();
      return OSCReceiver::connect(mInPort);
   }

   return false;
}

string OscController::GetControlTooltip(MidiMessageType type, int control)
{
   if (type == kMidiMessage_Control && control >= 0 && control < mOscMap.size())
      return mOscMap[control].mAddress;
   return "[unmapped]";
}

void OscController::SendValue(int page, int control, float value, bool forceNoteOn /*= false*/, int channel /*= -1*/)
{
   if (!mConnected)
      return;
   
   for (int i=0; i<mOscMap.size(); ++i)
   {
      if (control == mOscMap[i].mControl)// && mOscMap[i].mLastChangedTime + 50 < gTime)
      {
         OSCMessage msg(mOscMap[i].mAddress.c_str());

         if (mOscMap[i].mIsFloat)
         {
            mOscMap[i].mFloatValue = value;
            msg.addFloat32(mOscMap[i].mFloatValue);
         }
         else
         {
            mOscMap[i].mIntValue = value*127;
            msg.addInt32(mOscMap[i].mIntValue);
         }
         
         if (mOutputConnected)
            mOscOut.send(msg);
      }
   }
}

void OscController::oscMessageReceived(const OSCMessage& msg)
{
   string address = msg.getAddressPattern().toString().toStdString();

   if (address == "/jockey/sync")
   {
      string outputAddress = msg[0].getString().toStdString();
      vector<string> tokens= ofSplitString(outputAddress, ":");
      if (tokens.size() == 2)
      {
         mOutAddress = tokens[0];
         mOutPort = ofToInt(tokens[1]);
         ConnectOutput();
      }
      return;
   }

   if (msg.size() == 0 || (!msg[0].isFloat32() && !msg[0].isInt32()))
      return;

   int mapIndex = FindControl(address);

   bool isNew = false;
   if (mapIndex == -1)  //create a new map entry
   {
      isNew = true;
      mapIndex = AddControl(address, msg[0].isFloat32());
   }

   MidiControl control;
   control.mControl = mOscMap[mapIndex].mControl;
   control.mDeviceName = "osccontroller";
   control.mChannel = 1;
   mOscMap[mapIndex].mLastChangedTime = gTime;
   if (mOscMap[mapIndex].mIsFloat)
   {
      assert(msg[0].isFloat32());
      mOscMap[mapIndex].mFloatValue = msg[0].getFloat32();
      control.mValue = mOscMap[mapIndex].mFloatValue * 127;
   }
   else
   {
      assert(msg[0].isInt32());
      mOscMap[mapIndex].mIntValue = msg[0].getInt32();
      control.mValue = mOscMap[mapIndex].mIntValue;
   }

   if (isNew)
   {
      MidiController* midiController = dynamic_cast<MidiController*>(mListener);
      if (midiController)
      {
         auto& layoutControl = midiController->GetLayoutControl(control.mControl, kMidiMessage_Control);
         layoutControl.mConnectionType = mOscMap[mapIndex].mIsFloat ? kControlType_Slider : kControlType_Direct;
      }
   }

   mListener->OnMidiControl(control);
}

int OscController::FindControl(string address)
{
   for (int i = 0; i < mOscMap.size(); ++i)
   {
      if (address == mOscMap[i].mAddress)
         return i;
   }

   return -1;
}

int OscController::AddControl(string address, bool isFloat)
{
   int existing = FindControl(address);
   if (existing != -1)
      return existing;

   OscMap entry;
   int mapIndex = mOscMap.size();
   entry.mControl = mapIndex;
   entry.mAddress = address;
   entry.mIsFloat = isFloat;
   mOscMap.push_back(entry);

   return mapIndex;
}

namespace
{
   const int kSaveStateRev = 1;
}

void OscController::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << (int)mOscMap.size();
   for (size_t i = 0; i < mOscMap.size(); ++i)
   {
      out << mOscMap[i].mControl;
      out << mOscMap[i].mAddress;
      out << mOscMap[i].mIsFloat;
      out << mOscMap[i].mFloatValue;
      out << mOscMap[i].mIntValue;
      out << mOscMap[i].mLastChangedTime;
   }
}

void OscController::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);

   int mapSize;
   in >> mapSize;
   mOscMap.resize(mapSize);
   for (size_t i = 0; i < mOscMap.size(); ++i)
   {
      in >> mOscMap[i].mControl;
      in >> mOscMap[i].mAddress;
      in >> mOscMap[i].mIsFloat;
      in >> mOscMap[i].mFloatValue;
      in >> mOscMap[i].mIntValue;
      in >> mOscMap[i].mLastChangedTime;
   }
}

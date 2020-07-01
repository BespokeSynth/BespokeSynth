//
//  MidiDevice.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#include "MidiDevice.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

#include <string.h>

MidiDevice::MidiDevice(MidiDeviceListener* listener)
   : mListener(listener)
   , mMidiOut(nullptr)
   , mOutputChannel(1)
   , mIsInputEnabled(false)
{
   mDeviceNameIn[0] = 0;
   mDeviceNameOut[0] = 0;
}

MidiDevice::~MidiDevice()
{
   AudioDeviceManager& deviceManager = TheSynth->GetGlobalManagers()->mDeviceManager;
   deviceManager.removeMidiInputCallback(mDeviceNameIn, this);
}

bool MidiDevice::ConnectInput(const char* name)
{
   AudioDeviceManager& deviceManager = TheSynth->GetGlobalManagers()->mDeviceManager;
   
   deviceManager.removeMidiInputCallback(mDeviceNameIn, this);
   
   StringCopy(mDeviceNameIn, name, 64);
   
   deviceManager.setMidiInputEnabled(mDeviceNameIn, true);
   deviceManager.addMidiInputCallback(mDeviceNameIn, this);
   
   mIsInputEnabled = TheSynth->GetGlobalManagers()->mDeviceManager.isMidiInputEnabled(mDeviceNameIn);
   
   TheSynth->AddMidiDevice(this);   //TODO(Ryan) need better place for this, but constructor is too early
   
   return true;
}

void MidiDevice::ConnectInput(int index)
{
   ConnectInput(MidiInput::getDevices()[index].toRawUTF8());
}

bool MidiDevice::ConnectOutput(const char* name, int channel /*= 1*/)
{
   bool found = false;
   StringArray devices = MidiOutput::getDevices();
   for (int i=0; i<devices.size(); ++i)
   {
      if (devices[i] == name)
      {
         ConnectOutput(i, channel);
         found = true;
         break;
      }
   }
   
   if (!found)
   {
      mMidiOut.reset();
      mMidiOut = nullptr;
      mDeviceNameOut[0] = 0;
   }
   
   return found;
}

void MidiDevice::ConnectOutput(int index, int channel /*= 1*/)
{
   mMidiOut.reset();
   mMidiOut = MidiOutput::openDevice(index);

   StringCopy(mDeviceNameOut, mMidiOut->getName().toRawUTF8(), 64);

   assert(channel > 0 && channel <= 16);
   mOutputChannel = channel;
}

bool MidiDevice::Reconnect()
{
   bool ret = false;
   if (strlen(mDeviceNameIn) > 0)
      ret = ConnectInput(mDeviceNameIn);
   if (strlen(mDeviceNameOut) > 0)
      ConnectOutput(mDeviceNameOut);
   return ret;
}

bool MidiDevice::IsInputConnected()
{
   return TheSynth->GetGlobalManagers()->mDeviceManager.isMidiInputEnabled(mDeviceNameIn);
}

vector<string> MidiDevice::GetPortList(bool forInput)
{
   vector<string> portList;
   
   if (forInput)
   {
      const StringArray input = MidiInput::getDevices();
      for (int i=0; i<input.size(); ++i)
         portList.push_back(input[i].toStdString());
   }
   else
   {
      const StringArray output = MidiOutput::getDevices();
      for (int i=0; i<output.size(); ++i)
         portList.push_back(output[i].toStdString());
   }
   
   return portList;
}

void MidiDevice::SendNote(int pitch, int velocity, bool forceNoteOn /*= false*/, int channel /*= -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;
      
      if (velocity > 0 || forceNoteOn)
         mMidiOut->sendMessageNow(MidiMessage::noteOn(channel, pitch, (uint8)velocity));
      else
         mMidiOut->sendMessageNow(MidiMessage::noteOff(channel, pitch));
   }
}

void MidiDevice::SendCC(int ctl, int value, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;
      
      mMidiOut->sendMessageNow(MidiMessage::controllerEvent(channel, ctl, value));
   }
}

void MidiDevice::SendAftertouch(int pressure, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;
      
      //TODO_PORT(Ryan) pitch number?
      mMidiOut->sendMessageNow(MidiMessage::aftertouchChange(channel, 0, pressure));
   }
}

void MidiDevice::SendPitchBend(int bend, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;
      
      mMidiOut->sendMessageNow(MidiMessage::pitchWheel(channel, bend));
   }
}

void MidiDevice::SendData(unsigned char a, unsigned char b, unsigned char c)
{
   if (mMidiOut)
   {
      mMidiOut->sendMessageNow(MidiMessage(a,b,c));
   }
}

void MidiDevice::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
   if (TheSynth->IsReady() == false)
      return;
   
   if (mListener)
   {
      MidiDevice::SendMidiMessage(mListener, mDeviceNameIn, message);
      
      if (gPrintMidiInput)
         ofLog() << mDeviceNameIn << " " << message.getDescription();
   }
}

//static
void MidiDevice::SendMidiMessage(MidiDeviceListener* listener, const char* deviceName, const MidiMessage& message)
{
   listener->OnMidi(message);
   
   if (message.isNoteOnOrOff())
   {
      MidiNote note;
      note.mDeviceName = deviceName;
      note.mPitch = message.getNoteNumber();
      if (message.isNoteOn())
         note.mVelocity = message.getVelocity();
      else
         note.mVelocity = 0;
      note.mChannel = message.getChannel();
      listener->OnMidiNote(note);
   }
   if (message.isController())
   {
      MidiControl control;
      control.mDeviceName = deviceName;
      control.mControl = message.getControllerNumber();
      control.mValue = message.getControllerValue();
      control.mChannel = message.getChannel();
      listener->OnMidiControl(control);
   }
   if (message.isProgramChange())
   {
      MidiProgramChange program;
      program.mDeviceName = deviceName;
      program.mProgram = message.getProgramChangeNumber();
      program.mChannel = message.getChannel();
      listener->OnMidiProgramChange(program);
   }
   if (message.isPitchWheel())
   {
      MidiPitchBend pitchBend;
      pitchBend.mDeviceName = deviceName;
      pitchBend.mValue = message.getPitchWheelValue();
      pitchBend.mChannel = message.getChannel();
      listener->OnMidiPitchBend(pitchBend);
   }
   if (message.isChannelPressure())
   {
      MidiPressure pressure;
      pressure.mDeviceName = deviceName;
      //TODO_PORT(Ryan) - is this correct for the pitch? does pitch have meaning for channel pressure messages?
      pressure.mPitch = message.getNoteNumber();
      pressure.mPressure = message.getChannelPressureValue();
      pressure.mChannel = message.getChannel();
      listener->OnMidiPressure(pressure);
   }
   if (message.isAftertouch())
   {
      MidiPressure pressure;
      pressure.mDeviceName = deviceName;
      pressure.mPitch = -1;
      pressure.mPressure = message.getAfterTouchValue();
      pressure.mChannel = message.getChannel();
      listener->OnMidiPressure(pressure);
   }
}

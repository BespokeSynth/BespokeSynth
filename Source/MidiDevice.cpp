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
//  MidiDevice.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#include "MidiDevice.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

using namespace juce;

namespace
{
   std::vector<MidiDevice*> sAllDevices;
   std::atomic<int> sNumAllInputDevices{ 0 };

   //inputs that "all input" should ignore because a module handles them itself, read from the midi thread
   juce::SpinLock sHiddenInputsLock;
   juce::StringArray sHiddenInputIdentifiers;

   //recently sent messages, so that "all input" can recognize them if a loopback port (IAC bus, loopMIDI, etc) echoes them back.
   //matching actual messages (rather than hiding every input that shares a name with an output) keeps hardware like a keystep working,
   //since many devices use the same name for their input and output ports.
   struct SentMessage
   {
      juce::uint32 mOutputNameHash{ 0 };
      juce::uint32 mHash{ 0 };
      int mSize{ 0 };
      double mTimeMs{ -99999 };
   };
   juce::SpinLock sSentMessagesLock;
   std::array<SentMessage, 256> sSentMessages;
   int sSentMessagesIndex{ 0 };
   std::atomic<double> sLastSentMessageTimeMs{ -99999 };
   constexpr double kEchoWindowMs = 1000;

   juce::uint32 HashBytes(const juce::uint8* data, size_t size)
   {
      juce::uint32 hash = 2166136261u; //fnv-1a (JenkinsHash stops at zero bytes, which midi is full of)
      for (size_t i = 0; i < size; ++i)
         hash = (hash ^ data[i]) * 16777619u;
      return hash;
   }

   juce::uint32 HashMidiBytes(const juce::MidiMessage& message)
   {
      return HashBytes(message.getRawData(), (size_t)message.getRawDataSize());
   }

   juce::uint32 HashName(const juce::String& name)
   {
      return HashBytes(reinterpret_cast<const juce::uint8*>(name.toRawUTF8()), name.getNumBytesAsUTF8());
   }

   bool IsEchoOfSentMessage(const juce::String& inputName, const juce::MidiMessage& message)
   {
      const double now = juce::Time::getMillisecondCounterHiRes();
      if (now - sLastSentMessageTimeMs.load() >= kEchoWindowMs)
         return false;

      const juce::uint32 nameHash = HashName(inputName); //a loopback port's input has the same name as its output
      const juce::uint32 hash = HashMidiBytes(message);
      const juce::SpinLock::ScopedLockType lock(sSentMessagesLock);
      for (auto& sent : sSentMessages)
      {
         if (sent.mHash == hash && sent.mOutputNameHash == nameHash && sent.mSize == message.getRawDataSize() && now - sent.mTimeMs < kEchoWindowMs)
         {
            sent.mTimeMs = -99999; //each sent message accounts for one echo
            return true;
         }
      }
      return false;
   }

   juce::Array<juce::MidiDeviceInfo> sConnectedInputDevices;

   //tells us right away when devices are plugged/unplugged, so a quick unplug and replug isn't missed between the periodic refreshes
   std::unique_ptr<juce::MidiDeviceListConnection> sDeviceListConnection;
   std::atomic<bool> sDeviceListChanged{ false };

   bool IsInputPresent(const juce::String& identifier)
   {
      for (auto& device : sConnectedInputDevices)
      {
         if (device.identifier == identifier)
            return true;
      }
      return false;
   }

   void RefreshConnectedInputDevices(bool immediate)
   {
      static double sLastUpdatedListTime = -9999;
      if (sLastUpdatedListTime + 5000 < gTime || immediate || sDeviceListChanged.exchange(false)) //refresh every 5 seconds as a fallback (getAvailableDevices() is slow on windows)
      {
         const auto previousDevices = sConnectedInputDevices;
         sConnectedInputDevices = MidiInput::getAvailableDevices();
         sLastUpdatedListTime = gTime;

         if (sNumAllInputDevices > 0)
         {
            //open everything for "all input", and close inputs that have been unplugged so they get reopened if they're plugged back in
            auto& deviceManager = TheSynth->GetAudioDeviceManager();
            for (auto& device : sConnectedInputDevices)
               deviceManager.setMidiInputDeviceEnabled(device.identifier, true);
            for (auto& device : previousDevices)
            {
               if (!IsInputPresent(device.identifier))
                  deviceManager.setMidiInputDeviceEnabled(device.identifier, false);
            }
         }
      }
   }
}

MidiDevice::MidiDevice(MidiDeviceListener* listener)
: mListener(listener)
{
   sAllDevices.push_back(this);

   if (sDeviceListConnection == nullptr)
      sDeviceListConnection = std::make_unique<juce::MidiDeviceListConnection>(juce::MidiDeviceListConnection::make([]
                                                                                                                    {
                                                                                                                       sDeviceListChanged = true;
                                                                                                                    }));
}

MidiDevice::~MidiDevice()
{
   DisconnectInput();
   RemoveFromVector(this, sAllDevices);
   if (sAllDevices.empty())
      sDeviceListConnection.reset(); //don't leave this for static destruction, after juce has shut down
   if (mMidiOut.get())
      mMidiOut->stopBackgroundThread();
}

bool MidiDevice::ConnectInput(const char* name)
{
   if (juce::String(name) == kAllInputsName)
   {
      ConnectAllInputs();
      return true;
   }

   DisconnectInput();

   bool found = false;
   auto devices = MidiInput::getAvailableDevices();
   for (int i = 0; i < devices.size(); ++i)
   {
      if (devices[i].name == name)
      {
         mDeviceInInfo = devices[i];
         found = true;
         break;
      }
   }

   if (!found)
   {
      mIsInputEnabled = false;
      return false;
   }

   auto& deviceManager = TheSynth->GetAudioDeviceManager();
   deviceManager.setMidiInputDeviceEnabled(mDeviceInInfo.identifier, true);
   deviceManager.addMidiInputDeviceCallback(mDeviceInInfo.identifier, this);

   mIsInputEnabled = deviceManager.isMidiInputDeviceEnabled(mDeviceInInfo.identifier);
   mHasInputConnected = true;

   if (mInputHiddenFromAllInputs)
      UpdateHiddenFromAllInputs();

   TheSynth->AddMidiDevice(this); //TODO(Ryan) need better place for this, but constructor is too early

   return mIsInputEnabled;
}

void MidiDevice::ConnectInput(int index)
{
   ConnectInput(MidiInput::getAvailableDevices()[index].name.toRawUTF8());
}

void MidiDevice::ConnectAllInputs()
{
   DisconnectInput();

   mDeviceInInfo.name = kAllInputsName;
   mDeviceInInfo.identifier = ""; //an empty identifier registers the callback for all enabled inputs
   mIsConnectedToAllInputs = true;
   mIsInputEnabled = true;
   mHasInputConnected = true;
   ++sNumAllInputDevices;

   TheSynth->GetAudioDeviceManager().addMidiInputDeviceCallback(mDeviceInInfo.identifier, this);

   RefreshConnectedInputDevices(K(immediate)); //opens all of the inputs
}

void MidiDevice::SetInputHiddenFromAllInputs(bool hidden)
{
   mInputHiddenFromAllInputs = hidden;
   UpdateHiddenFromAllInputs();
}

//static
void MidiDevice::UpdateHiddenFromAllInputs()
{
   juce::StringArray identifiers;
   for (auto* device : sAllDevices)
   {
      if (device->mInputHiddenFromAllInputs && device->mHasInputConnected)
         identifiers.add(device->mDeviceInInfo.identifier);
   }

   const juce::SpinLock::ScopedLockType lock(sHiddenInputsLock);
   sHiddenInputIdentifiers.swapWith(identifiers);
}

bool MidiDevice::IsHiddenFromAllInputs(const juce::String& identifier, const juce::String& name, const juce::MidiMessage& message) const
{
   {
      const juce::SpinLock::ScopedLockType lock(sHiddenInputsLock);
      if (sHiddenInputIdentifiers.contains(identifier))
         return true;
   }

   return IsEchoOfSentMessage(name, message);
}

void MidiDevice::RememberSentMessage(const juce::MidiMessage& message)
{
   if (sNumAllInputDevices == 0)
      return; //nobody is listening for echoes

   const double now = juce::Time::getMillisecondCounterHiRes();
   const juce::SpinLock::ScopedLockType lock(sSentMessagesLock);
   auto& sent = sSentMessages[sSentMessagesIndex];
   sSentMessagesIndex = (sSentMessagesIndex + 1) % (int)sSentMessages.size();
   sent.mOutputNameHash = mOutputNameHash;
   sent.mHash = HashMidiBytes(message);
   sent.mSize = message.getRawDataSize();
   sent.mTimeMs = now;
   sLastSentMessageTimeMs = now;
}

void MidiDevice::SendMessageNow(const juce::MidiMessage& message)
{
   RememberSentMessage(message);
   mMidiOut->sendMessageNow(message);
}

void MidiDevice::SendBlock(const juce::MidiMessage& message, int sampleNumber)
{
   RememberSentMessage(message);
   juce::MidiBuffer midiBuffer;
   midiBuffer.addEvent(message, sampleNumber);
   mMidiOut->sendBlockOfMessages(midiBuffer, Time::getMillisecondCounter(), gSampleRate);
}

void MidiDevice::DisableInputIfUnused(const juce::String& identifier)
{
   if (IsInputPresent(identifier)) //if it's been unplugged, always close it so that it can be reopened later
   {
      for (auto* device : sAllDevices)
      {
         if (device != this && device->mHasInputConnected && (device->mIsConnectedToAllInputs || device->mDeviceInInfo.identifier == identifier))
            return;
      }
   }

   TheSynth->GetAudioDeviceManager().setMidiInputDeviceEnabled(identifier, false);
}

bool MidiDevice::ConnectOutput(const char* name, int channel /*= 1*/)
{
   DisconnectOutput();

   bool found = false;
   auto devices = MidiOutput::getAvailableDevices();
   for (int i = 0; i < devices.size(); ++i)
   {
      if (devices[i].name == name)
      {
         found = ConnectOutput(i, channel);
         break;
      }
   }

   if (!found)
   {
      TheSynth->LogEvent("Failed to connect to Midi Output: " + std::string(name), kLogEventType_Error);
      DisconnectOutput();
   }

   return found;
}

bool MidiDevice::ConnectOutput(int index, int channel /*= 1*/)
{
   mMidiOut.reset();
   mMidiOut = MidiOutput::openDevice(MidiOutput::getAvailableDevices()[index].identifier);
   if (mMidiOut)
   {
      mMidiOut->startBackgroundThread();
      mDeviceOutInfo = mMidiOut->getDeviceInfo();
      mOutputNameHash = HashName(mDeviceOutInfo.name);

      assert(channel > 0 && channel <= 16);
      mOutputChannel = channel;
      return true;
   }

   return false;
}

void MidiDevice::DisconnectInput()
{
   if (!mHasInputConnected)
      return;
   mHasInputConnected = false;

   auto& deviceManager = TheSynth->GetAudioDeviceManager();
   deviceManager.removeMidiInputDeviceCallback(mDeviceInInfo.identifier, this);
   if (mIsConnectedToAllInputs)
   {
      mIsConnectedToAllInputs = false;
      --sNumAllInputDevices;
      for (auto& device : sConnectedInputDevices)
         DisableInputIfUnused(device.identifier);
   }
   else
   {
      DisableInputIfUnused(mDeviceInInfo.identifier);
   }

   if (mInputHiddenFromAllInputs)
      UpdateHiddenFromAllInputs();
}

void MidiDevice::DisconnectOutput()
{
   if (mMidiOut)
      mMidiOut->stopBackgroundThread();
   mMidiOut.reset();
   mMidiOut = nullptr;
   mDeviceOutInfo.name = "";
   mDeviceOutInfo.identifier = "";
   mOutputNameHash = 0;
}

bool MidiDevice::Reconnect()
{
   bool ret = false;
   if (mDeviceInInfo.identifier.isNotEmpty())
      ret = ConnectInput(mDeviceInInfo.name.toRawUTF8());
   if (mDeviceOutInfo.identifier.isNotEmpty())
      ConnectOutput(mDeviceOutInfo.name.toRawUTF8());
   return ret;
}

bool MidiDevice::IsInputConnected(bool immediate)
{
   RefreshConnectedInputDevices(immediate);

   if (mIsConnectedToAllInputs)
      return true; //always available, even if nothing is plugged in yet

   return IsInputPresent(mDeviceInInfo.identifier);
}

std::vector<std::string> MidiDevice::GetPortList(bool forInput)
{
   std::vector<std::string> portList;

   if (forInput)
   {
      for (auto& device : MidiInput::getAvailableDevices())
         portList.push_back(device.name.toStdString());
   }
   else
   {
      for (auto& device : MidiOutput::getAvailableDevices())
         portList.push_back(device.name.toStdString());
   }

   return portList;
}

void MidiDevice::SendNote(double time, int pitch, int velocity, bool forceNoteOn, int channel)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;

      int sampleNumber = (time - gTime) * gSampleRateMs;

      SendBlock((velocity > 0 || forceNoteOn) ? MidiMessage::noteOn(channel, pitch, (uint8)velocity) : MidiMessage::noteOff(channel, pitch), sampleNumber);
   }
}

void MidiDevice::SendCC(int ctl, int value, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;

      SendMessageNow(MidiMessage::controllerEvent(channel, ctl, value));
   }
}

void MidiDevice::SendAftertouch(int pressure, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;

      //TODO_PORT(Ryan) pitch number?
      SendMessageNow(MidiMessage::aftertouchChange(channel, 0, pressure));
   }
}

void MidiDevice::SendProgramChange(int program, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;

      SendMessageNow(MidiMessage::programChange(channel, program));
   }
}

void MidiDevice::SendPitchBend(int bend, int channel /* = -1*/)
{
   if (mMidiOut)
   {
      if (channel == -1)
         channel = mOutputChannel;

      SendMessageNow(MidiMessage::pitchWheel(channel, bend));
   }
}

void MidiDevice::SendSysEx(std::string data)
{
   if (mMidiOut)
   {
      SendMessageNow(MidiMessage::createSysExMessage(data.c_str(), (int)data.length()));
   }
}

void MidiDevice::SendData(unsigned char a, unsigned char b, unsigned char c)
{
   if (mMidiOut)
   {
      SendMessageNow(MidiMessage(a, b, c));
   }
}

void MidiDevice::SendMessage(double time, juce::MidiMessage message)
{
   if (mMidiOut)
   {
      int sampleNumber = (time - gTime) * gSampleRateMs;
      SendBlock(message, sampleNumber);
   }
}

void MidiDevice::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
   if (TheSynth->IsReady() == false)
      return;

   juce::String name = mDeviceInInfo.name;
   if (mIsConnectedToAllInputs && source != nullptr)
   {
      name = source->getName();
      if (IsHiddenFromAllInputs(source->getIdentifier(), name, message))
         return;
   }

   if (mListener)
   {
      MidiDevice::SendMidiMessage(mListener, name.toRawUTF8(), message);

      if (gPrintMidiInput)
         ofLog() << name << " " << message.getDescription();
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
      note.mTimestampMs = message.getTimeStamp() * 1000; //message.getTimeStamp() is equivalent to Time::getMillisecondCounter() / 1000.0 (see juce_MidiDevices.h)
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
   /*if (message.isMidiClock())
   {
      static double sLastTime = -1;
      static std::array<float, 40> sTempos;
      static int sTempoIdx = 0;
      if (sLastTime > 0)
      {
         double deltaSeconds = (message.getTimeStamp() - sLastTime);
         double pulsesPerSecond = 1 / deltaSeconds;
         double beatsPerSecond = pulsesPerSecond / 24;
         double instantTempo = beatsPerSecond * 60;
         sTempos[sTempoIdx] = instantTempo;
         sTempoIdx = (sTempoIdx + 1) % sTempos.size();
         double avgTempo = 0;
         for (auto& tempo : sTempos)
            avgTempo += tempo;
         avgTempo /= sTempos.size();
         if (sTempoIdx == 0)
            ofLog() << avgTempo;
      }
      sLastTime = message.getTimeStamp();
   }
   if (message.isMidiStart())
   {
      ofLog() << "midi start";
   }
   if (message.isMidiStop())
   {
      ofLog() << "midi stop";
   }
   if (message.isMidiContinue())
   {
      ofLog() << "midi continue";
   }
   if (message.isSongPositionPointer())
   {
      ofLog() << "midi position pointer " << ofToString(message.getSongPositionPointerMidiBeat());
   }
   if (message.isQuarterFrame())
   {
      ofLog() << "midi quarter frame " << ofToString(message.getQuarterFrameValue()) << " " << ofToString(message.getQuarterFrameSequenceNumber());
   }*/
}

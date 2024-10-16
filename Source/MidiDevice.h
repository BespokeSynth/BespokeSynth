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
//  MidiDevice.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#pragma once

#include "ModularSynth.h"

#include "juce_audio_devices/juce_audio_devices.h"

struct MidiNote
{
   const char* mDeviceName;
   double mTimestampMs{ 0 };
   int mPitch{ 0 };
   float mVelocity{ 0 }; //0-127
   int mChannel{ -1 };
};

struct MidiControl
{
   const char* mDeviceName;
   int mControl{ 0 };
   float mValue{ 0 };
   int mChannel{ -1 };
};

struct MidiProgramChange
{
   const char* mDeviceName;
   int mProgram{ 0 };
   int mChannel{ -1 };
};

struct MidiPitchBend
{
   const char* mDeviceName;
   float mValue{ 0 };
   int mChannel{ -1 };
};

struct MidiPressure
{
   const char* mDeviceName;
   int mPitch{ 0 };
   float mPressure{ 0 };
   int mChannel{ -1 };
};

class MidiDeviceListener
{
public:
   virtual ~MidiDeviceListener() {}
   virtual void ControllerPageSelected() {}
   virtual void OnMidiNote(MidiNote& note) = 0;
   virtual void OnMidiControl(MidiControl& control) = 0;
   virtual void OnMidiProgramChange(MidiProgramChange& program) {}
   virtual void OnMidiPitchBend(MidiPitchBend& pitchBend) {}
   virtual void OnMidiPressure(MidiPressure& pressure) {}
   virtual void OnMidi(const juce::MidiMessage& message) {}
};

class MidiDevice : public juce::MidiInputCallback
{
public:
   MidiDevice(MidiDeviceListener* listener);
   virtual ~MidiDevice();

   bool ConnectInput(const char* name);
   void ConnectInput(int index);
   bool ConnectOutput(const char* name, int channel = 1);
   bool ConnectOutput(int index, int channel = 1);
   void DisconnectInput();
   void DisconnectOutput();
   bool Reconnect();
   bool IsInputConnected(bool immediate);

   const char* Name() { return mIsInputEnabled ? mDeviceInInfo.name.toRawUTF8() : mDeviceOutInfo.name.toRawUTF8(); }

   std::vector<std::string> GetPortList(bool forInput);

   void SendNote(double time, int pitch, int velocity, bool forceNoteOn, int channel);
   void SendCC(int ctl, int value, int channel = -1);
   void SendAftertouch(int pressure, int channel = -1);
   void SendProgramChange(int program, int channel = -1);
   void SendPitchBend(int bend, int channel = -1);
   void SendSysEx(std::string data);
   void SendData(unsigned char a, unsigned char b, unsigned char c);
   void SendMessage(double time, juce::MidiMessage message);

   static void SendMidiMessage(MidiDeviceListener* listener, const char* deviceName, const juce::MidiMessage& message);

   static constexpr float kPitchBendCenter{ 8192.0f };
   static constexpr float kPitchBendMax{ 16320.0f };

private:
   void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

   juce::MidiDeviceInfo mDeviceInInfo;
   juce::MidiDeviceInfo mDeviceOutInfo;

   std::unique_ptr<juce::MidiOutput> mMidiOut{ nullptr };
   MidiDeviceListener* mListener{ nullptr };
   int mOutputChannel{ 1 };
   bool mIsInputEnabled{ false };
};

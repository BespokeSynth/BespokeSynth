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

#ifndef __additiveSynth__MidiDevice__
#define __additiveSynth__MidiDevice__

#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

#include "juce_audio_devices/juce_audio_devices.h"

struct MidiNote
{
   const char* mDeviceName;
   double mTimestampMs;
   int mPitch;
   float mVelocity; //0-127
   int mChannel;
};

struct MidiControl
{
   const char* mDeviceName;
   int mControl;
   float mValue;
   int mChannel;
};

struct MidiProgramChange
{
   const char* mDeviceName;
   int mProgram;
   int mChannel;
};

struct MidiPitchBend
{
   const char* mDeviceName;
   float mValue;
   int mChannel;
};

struct MidiPressure
{
   const char* mDeviceName;
   int mPitch;
   float mPressure;
   int mChannel;
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
   void ConnectOutput(int index, int channel = 1);
   void DisconnectInput();
   void DisconnectOutput();
   bool Reconnect();
   bool IsInputConnected(bool immediate);

   const char* Name() { return mIsInputEnabled ? mDeviceNameIn.toRawUTF8() : mDeviceNameOut.toRawUTF8(); }
   
   std::vector<std::string> GetPortList(bool forInput);
   
   void SendNote(double time, int pitch, int velocity, bool forceNoteOn, int channel);
   void SendCC(int ctl, int value, int channel = -1);
   void SendAftertouch(int pressure, int channel = -1);
   void SendProgramChange(int program, int channel = -1);
   void SendPitchBend(int bend, int channel = -1);
   void SendData(unsigned char a, unsigned char b, unsigned char c);
   void SendMessage(double time, juce::MidiMessage message);
   
   static void SendMidiMessage(MidiDeviceListener* listener, const char* deviceName, const juce::MidiMessage& message);
   
private:
   void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
   
   juce::String mDeviceNameIn;
   juce::String mDeviceNameOut;
   
   std::unique_ptr<juce::MidiOutput> mMidiOut;
   MidiDeviceListener* mListener;
   int mOutputChannel;
   bool mIsInputEnabled;
};

#endif /* defined(__additiveSynth__MidiDevice__) */

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
//  INoteReceiver.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#pragma once

#include "ModulationChain.h"

namespace juce
{
   class MidiMessage;
}

struct NoteMessage
{
   NoteMessage()
   {
   }

   NoteMessage(double _time, int _pitch, int _velocity, int _voiceIdx = -1, ModulationParameters _modulation = ModulationParameters())
   : time(_time)
   , pitch(_pitch)
   , velocity(_velocity)
   , voiceIdx(_voiceIdx)
   , modulation(_modulation)
   {
   }

   NoteMessage MakeClone()
   {
      NoteMessage clone;
      clone.time = time;
      clone.pitch = pitch;
      clone.velocity = velocity;
      clone.voiceIdx = voiceIdx;
      clone.modulation = modulation;
      //generate new note id here

      return clone;
   }

   NoteMessage MakeNoteOff()
   {
      return NoteMessage(time, pitch, 0, voiceIdx, modulation);
   }

   double time{ 0 };
   int pitch{ 0 };
   float velocity{ 0 };
   int voiceIdx{ -1 };
   ModulationParameters modulation;
};

class INoteReceiver
{
public:
   virtual ~INoteReceiver() {}
   virtual void PlayNote(NoteMessage note) = 0;
   virtual void SendPressure(int pitch, int pressure) {}
   virtual void SendCC(int control, int value, int voiceIdx = -1) = 0;
   virtual void SendMidi(const juce::MidiMessage& message) {}
};

class NoteInputBuffer
{
public:
   NoteInputBuffer(INoteReceiver* receiver);
   void Process(double time);
   void QueueNote(NoteMessage note);
   static bool IsTimeWithinFrame(double time);

private:
   static const int kBufferSize = 50;
   NoteMessage mBuffer[kBufferSize];
   INoteReceiver* mReceiver{ nullptr };
};

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
/*
  ==============================================================================

    INoteReceiver.cpp
    Created: 17 May 2020 10:59:27pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "INoteReceiver.h"
#include "Profiler.h"

NoteInputBuffer::NoteInputBuffer(INoteReceiver* receiver)
: mReceiver(receiver)
{
   for (int i = 0; i < kBufferSize; ++i)
      mBuffer[i].time = -1;
}

void NoteInputBuffer::Process(double time)
{
   PROFILER(NoteInputBuffer);

   //process note offs first
   for (int i = 0; i < kBufferSize; ++i)
   {
      if (mBuffer[i].time != -1 && mBuffer[i].velocity == 0 &&
          IsTimeWithinFrame(mBuffer[i].time))
      {
         NoteMessage& element = mBuffer[i];
         mReceiver->PlayNote(element);
         mBuffer[i].time = -1;
      }
   }

   //now process note ons
   for (int i = 0; i < kBufferSize; ++i)
   {
      if (mBuffer[i].time != -1 && mBuffer[i].velocity != 0 &&
          IsTimeWithinFrame(mBuffer[i].time))
      {
         NoteMessage& element = mBuffer[i];
         mReceiver->PlayNote(element);
         mBuffer[i].time = -1;
      }
   }
}

void NoteInputBuffer::QueueNote(NoteMessage note)
{
   for (int i = 0; i < kBufferSize; ++i)
   {
      if (mBuffer[i].time == -1)
      {
         mBuffer[i].time = note.time;
         NoteMessage& element = mBuffer[i];
         element.pitch = note.pitch;
         element.velocity = note.velocity;
         element.voiceIdx = note.voiceIdx;
         element.modulation = note.modulation;
         break;
      }
   }
}

//static
bool NoteInputBuffer::IsTimeWithinFrame(double time)
{
   return time <= NextBufferTime(false);
}

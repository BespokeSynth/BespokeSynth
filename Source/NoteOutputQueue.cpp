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
//  INoteSource.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/14/12.
//
//

#include "NoteOutputQueue.h"
#include "INoteSource.h"
#include "ModularSynth.h"

void NoteOutputQueue::QueuePlayNote(NoteOutput* target, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PendingNoteOutput output;
   output.target = target;
   output.time = time;
   output.pitch = pitch;
   output.velocity = velocity;
   output.voiceIdx = voiceIdx;
   output.modulation = modulation;
   mQueue.enqueue(output);
}

void NoteOutputQueue::QueueFlush(NoteOutput* target, double time)
{
   PendingNoteOutput output;
   output.target = target;
   output.isFlush = true;
   output.time = time;
   mQueue.enqueue(output);
}

void NoteOutputQueue::Process()
{
   assert(IsAudioThread());

   PendingNoteOutput output;
   while (true)
   {
      bool hasData = mQueue.try_dequeue(output);
      if (!hasData)
         break;

      if (output.isFlush)
      {
         output.target->Flush(output.time);
      }
      else
      {
         //ofLog() << "playing queued note " << output.time << " " << output.pitch << " " << output.velocity << " " << gTime;
         output.target->PlayNoteInternal(output.time, output.pitch, output.velocity, output.voiceIdx, output.modulation, false);
      }
   }
}

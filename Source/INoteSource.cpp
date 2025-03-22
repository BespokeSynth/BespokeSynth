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

#include "INoteSource.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Scale.h"
#include "PatchCableSource.h"
#include "Profiler.h"
#include "NoteOutputQueue.h"

void NoteOutput::PlayNote(NoteMessage note)
{
   ResetStackDepth();
   PlayNoteInternal(note, false);
}

void NoteOutput::PlayNoteInternal(NoteMessage note, bool isFromMainThreadAndScheduled)
{
   if (!IsAudioThread())
   {
      if (!isFromMainThreadAndScheduled) //if we specifically scheduled this ahead of time, there's no need to make adjustments. otherwise, account for immediately requesting a note from the non-audio thread
      {
         note.time += TheTransport->GetEventLookaheadMs();
         if (note.velocity == 0)
            note.time += gBufferSizeMs; //1 buffer later, to make sure notes get cleared
      }
      TheSynth->GetNoteOutputQueue()->QueuePlayNote(this, note);
      return;
   }

   const int kMaxDepth = 100;
   if (mStackDepth > kMaxDepth)
   {
      TheSynth->LogEvent("note chain hit max stack depth", kLogEventType_Error);
      return; //avoid stack overflow
   }
   ++mStackDepth;

   if (note.pitch >= 0 && note.pitch <= 127)
   {
      for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
         noteReceiver->PlayNote(note);

      if (note.velocity > 0)
      {
         mNoteOnTimes[note.pitch] = note.time;
         mNotes[note.pitch] = true;
      }
      else
      {
         if (note.time > mNoteOnTimes[note.pitch])
            mNotes[note.pitch] = false;
      }

      mNoteSource->GetPatchCableSource()->AddHistoryEvent(note.time, HasHeldNotes());
   }
}

void NoteOutput::SendPressure(int pitch, int pressure)
{
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
      noteReceiver->SendPressure(pitch, pressure);
}

void NoteOutput::SendCC(int control, int value, int voiceIdx)
{
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
      noteReceiver->SendCC(control, value, voiceIdx);
}

void NoteOutput::SendMidi(const juce::MidiMessage& message)
{
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
      noteReceiver->SendMidi(message);
}

bool NoteOutput::HasHeldNotes()
{
   for (int i = 0; i < 128; ++i)
   {
      if (mNotes[i])
         return true;
   }
   return false;
}

std::list<int> NoteOutput::GetHeldNotesList()
{
   std::list<int> notes;
   for (int i = 0; i < 128; ++i)
   {
      if (mNotes[i])
         notes.push_back(i);
   }
   return notes;
}

void NoteOutput::Flush(double time)
{
   if (!IsAudioThread())
   {
      TheSynth->GetNoteOutputQueue()->QueueFlush(this, time + TheTransport->GetEventLookaheadMs() + gBufferSizeMs); //include event lookahead, and make it 1 buffer later, to make sure notes get cleared
      return;
   }

   bool flushed = false;

   for (int i = 0; i < 128; ++i)
   {
      if (mNotes[i])
      {
         for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
         {
            noteReceiver->PlayNote(NoteMessage(time, i, 0));
            noteReceiver->PlayNote(NoteMessage(time + Transport::sEventEarlyMs, i, 0));
         }
         flushed = true;
         mNotes[i] = false;
      }
   }

   if (flushed)
      mNoteSource->GetPatchCableSource()->AddHistoryEvent(time, false);
}

void INoteSource::PlayNoteOutput(NoteMessage note, bool isFromMainThreadAndScheduled)
{
   PROFILER(INoteSourcePlayOutput);

   if (note.time < gTime && note.velocity > 0)
      ofLog() << "Calling PlayNoteOutput() with a time in the past!  " << ofToString(note.time / 1000) << " < " << ofToString(gTime / 1000);

   if (!mInNoteOutput)
      mNoteOutput.ResetStackDepth();
   mInNoteOutput = true;
   mNoteOutput.PlayNoteInternal(note, isFromMainThreadAndScheduled);
   mInNoteOutput = false;
}

void INoteSource::SendCCOutput(int control, int value, int voiceIdx /*=-1*/)
{
   mNoteOutput.SendCC(control, value, voiceIdx);
}

void INoteSource::PreRepatch(PatchCableSource* cableSource)
{
   mNoteOutput.Flush(NextBufferTime(false));
}

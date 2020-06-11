//
//  INoteSource.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/14/12.
//
//

#include "INoteSource.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "ModularSynth.h"
#include "Scale.h"
#include "PatchCableSource.h"
#include "Profiler.h"

void NoteOutput::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (pitch >= 0 && pitch <= 127)
   {
      for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
         noteReceiver->PlayNote(time,pitch,velocity,voiceIdx,modulation);

      if (velocity>0)
      {
         mNoteOnTimes[pitch] = time;
         mNotes[pitch] = true;
      }
      else
      {
         if (time > mNoteOnTimes[pitch])
            mNotes[pitch] = false;
      }
      
      mNoteSource->GetPatchCableSource()->AddHistoryEvent(time, HasHeldNotes());
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

void NoteOutput::SendMidi(const MidiMessage& message)
{
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
      noteReceiver->SendMidi(message);
}

bool NoteOutput::HasHeldNotes()
{
   for (int i=0; i<128; ++i)
   {
      if (mNotes[i])
         return true;
   }
   return false;
}

list<int> NoteOutput::GetHeldNotesList()
{
   list<int> notes;
   for (int i=0; i<128; ++i)
   {
      if (mNotes[i])
         notes.push_back(i);
   }
   return notes;
}

void NoteOutput::Flush(double time)
{
   bool flushed = false;
   
   for (int i=0; i<128; ++i)
   {
      if (mNotes[i])
      {
         for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
         {
            noteReceiver->PlayNote(time,i,0);
            noteReceiver->PlayNote(time+Transport::sEventEarlyMs,i,0);
         }
         flushed = true;
         mNotes[i] = false;
      }
   }
   
   if (flushed)
      mNoteSource->GetPatchCableSource()->AddHistoryEvent(time, false);
}

void NoteOutput::FlushTarget(double time, INoteReceiver* target)
{
   if (target)
   {
      for (int i=0; i<128; ++i)
      {
         if (mNotes[i])
            target->PlayNote(time,i,0);
      }
   }
}

void INoteSource::PlayNoteOutput(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PROFILER(INoteSourcePlayOutput);
   if (time < gTime)
      ofLog() << "Calling PlayNoteOutput() with a time in the past!  " << ofToString(time) << " < " << ofToString(gTime);
   
   mNoteOutput.PlayNote(time, pitch, velocity, voiceIdx, modulation);
   
   if (mIsNoteOrigin)
   {
      //update visual info for waveform display
      bool* heldNotes = mNoteOutput.GetNotes();
      for (int i=0; i<128; ++i)
      {
         if (heldNotes[i])
         {
            gVizFreq = MAX(1,TheScale->PitchToFreq(i-12));
            break;
         }
      }
   }
}

void INoteSource::SendCCOutput(int control, int value, int voiceIdx /*=-1*/)
{
   mNoteOutput.SendCC(control, value, voiceIdx);
}

void INoteSource::PreRepatch(PatchCableSource* cableSource)
{
   mNoteOutput.Flush(gTime);
}

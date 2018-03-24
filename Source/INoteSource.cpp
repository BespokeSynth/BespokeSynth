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

void NoteHistory::AddEvent(double time, bool on)
{
   Lock("AddEvent");
   
   NoteHistoryEvent hist;
   hist.mTime = time;
   hist.mOn = on;
   mHistory.push_front(hist);
   
   bool deleteRest = false;
   for (NoteHistoryList::iterator i = mHistory.begin(); i != mHistory.end();)
   {
      if (deleteRest)
      {
         i = mHistory.erase(i);
      }
      else if ((*i).mTime < gTime - 5000)
      {
         deleteRest = true;
         ++i;
      }
      else
      {
         ++i;
      }
   }
   
   Unlock();
}

bool NoteHistory::CurrentlyOn()
{
   bool on = false;
   Lock("CurrentlyOn");
   if (!mHistory.empty())
   {
      const NoteHistoryEvent& note = *(mHistory.begin());
      on = note.mOn;
   }
   Unlock();
   return on;
}

void NoteOutput::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
      noteReceiver->PlayNote(time,pitch,velocity,voiceIdx,modulation);

   mNotesMutex.lock();
   if (velocity>0)
   {
      if (!ListContains(pitch, mNotes))
         mNotes.push_front(pitch);
   }
   else
   {
      mNotes.remove(pitch);
   }
   
   mNoteHistory.AddEvent(time, mNotes.size() > 0);
   mNotesMutex.unlock();
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

void NoteOutput::Flush()
{
   mNotesMutex.lock();
   for (auto noteReceiver : mNoteSource->GetPatchCableSource()->GetNoteReceivers())
   {
      for (list<int>::iterator iter = mNotes.begin(); iter != mNotes.end(); ++iter)
         noteReceiver->PlayNote(gTime,*iter,0);
   }
   mNotes.clear();
   
   mNoteHistory.AddEvent(gTime, false);
   mNotesMutex.unlock();
}

void NoteOutput::FlushTarget(INoteReceiver* target)
{
   if (target)
   {
      mNotesMutex.lock();
      for (list<int>::iterator iter = mNotes.begin(); iter != mNotes.end(); ++iter)
         target->PlayNote(gTime,*iter,0);
      mNotesMutex.unlock();
   }
}

void INoteSource::PlayNoteOutput(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   mNoteOutput.PlayNote(time, pitch, velocity, voiceIdx, modulation);
   
   if (mIsNoteOrigin)
   {
      //update visual info for waveform display
      list<int> heldNotes = mNoteOutput.GetHeldNotes();
      if (heldNotes.size() > 0)
      {
         int lowestPitch = 999;
         for (list<int>::iterator iter = heldNotes.begin();
              iter != heldNotes.end();
              ++iter)
         {
            if (*iter < lowestPitch)
               lowestPitch = *iter;
         }
         lowestPitch -= 12;
         
         gVizFreq = MAX(1,TheScale->PitchToFreq(lowestPitch));
      }
   }
}

void INoteSource::SendCCOutput(int control, int value, int voiceIdx /*=-1*/)
{
   mNoteOutput.SendCC(control, value);
}

void INoteSource::PreRepatch(PatchCableSource* cableSource)
{
   mNoteOutput.Flush();
}

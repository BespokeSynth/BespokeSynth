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
//  INoteSource.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#pragma once

#include "INoteReceiver.h"
#include "IPatchable.h"

class IDrawableModule;

class INoteSource;

//intercepts notes to keep track of them for visualization
class NoteOutput : public INoteReceiver
{
public:
   explicit NoteOutput(INoteSource* source)
   : mNoteSource(source)
   {}

   void Flush(double time);

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const juce::MidiMessage& message) override;

   void PlayNoteInternal(NoteMessage note, bool isFromMainThreadAndScheduled);

   void ResetStackDepth() { mStackDepth = 0; }
   bool* GetNotes() { return mNotes; }
   bool HasHeldNotes();
   std::list<int> GetHeldNotesList();

private:
   bool mNotes[128]{};
   double mNoteOnTimes[128]{};
   INoteSource* mNoteSource{ nullptr };
   int mStackDepth{ 0 };
};

class INoteSource : public virtual IPatchable
{
public:
   INoteSource()
   : mNoteOutput(this)
   {}
   virtual ~INoteSource() {}
   void PlayNoteOutput(NoteMessage note, bool isFromMainThreadAndScheduled = false);
   void SendCCOutput(int control, int value, int voiceIdx = -1);

   //IPatchable
   void PreRepatch(PatchCableSource* cableSource) override;

protected:
   NoteOutput mNoteOutput;
   bool mInNoteOutput{ false };
};

class AdditionalNoteCable : public INoteSource
{
public:
   void SetPatchCableSource(PatchCableSource* cable) { mCable = cable; }
   PatchCableSource* GetPatchCableSource(int index = 0) override { return mCable; }
   void Flush(double time) { mNoteOutput.Flush(time); }

private:
   PatchCableSource* mCable{ nullptr };
};

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

#ifndef modularSynth_INoteSource_h
#define modularSynth_INoteSource_h

#include "OpenFrameworksPort.h"
#include "INoteReceiver.h"
#include "IPatchable.h"

class IDrawableModule;

class INoteSource;

//intercepts notes to keep track of them for visualization
class NoteOutput : public INoteReceiver
{
public:
   NoteOutput(INoteSource* source) : mNoteSource(source), mStackDepth(0) { std::memset(mNotes, 0, 128*sizeof(bool)); std::memset(mNoteOnTimes, 0, 128*sizeof(double)); }
   
   void Flush(double time);
   void FlushTarget(double time, INoteReceiver* target);
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const juce::MidiMessage& message) override;
   
   void PlayNoteInternal(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());

   void ResetStackDepth() { mStackDepth = 0; }
   bool* GetNotes() { return mNotes; }
   bool HasHeldNotes();
   list<int> GetHeldNotesList();
private:
   bool mNotes[128];
   double mNoteOnTimes[128];
   INoteSource* mNoteSource;
   int mStackDepth;
};

class INoteSource : public virtual IPatchable
{
public:
   INoteSource() : mNoteOutput(this), mInNoteOutput(false) {}
   virtual ~INoteSource() {}
   void PlayNoteOutput(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());
   void SendCCOutput(int control, int value, int voiceIdx = -1);
   
   //IPatchable
   void PreRepatch(PatchCableSource* cableSource) override;
protected:
   NoteOutput mNoteOutput;
   bool mInNoteOutput;
};

class AdditionalNoteCable : public INoteSource
{
public:
   void SetPatchCableSource(PatchCableSource* cable) { mCable = cable; }
   PatchCableSource* GetPatchCableSource(int index=0) override { return mCable; }
   void Flush(double time) { mNoteOutput.Flush(time); }
private:
   PatchCableSource* mCable;
};

#endif

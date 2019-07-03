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
   NoteOutput(INoteSource* source) : mNoteSource(source) {}
   
   void Flush();
   void FlushTarget(INoteReceiver* target);
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   void SendMidi(const MidiMessage& message) override;

   list<int> GetHeldNotes() { Poco::FastMutex::ScopedLock lock(mNotesMutex); return mNotes; }
   list<int> GetHeldNotesAudioThread() { return mNotes; }
private:
   list<int> mNotes;
   ofMutex mNotesMutex;
   INoteSource* mNoteSource;
};

class INoteSource : public virtual IPatchable
{
public:
   INoteSource() : mIsNoteOrigin(false), mNoteOutput(this) {}
   virtual ~INoteSource() {}
   NoteOutput* GetNoteOutput() { return &mNoteOutput; }
   void PlayNoteOutput(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());
   void SendCCOutput(int control, int value, int voiceIdx = -1);
   void SetIsNoteOrigin(bool origin) { mIsNoteOrigin = origin; }
   void SetTargets(string targets);
   
   //IPatchable
   void PreRepatch(PatchCableSource* cableSource) override;
protected:
   NoteOutput mNoteOutput;
private:
   bool mIsNoteOrigin;
};

#endif

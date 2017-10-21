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
#include "NamedMutex.h"
#include "IPatchable.h"

class IDrawableModule;

#define NOTE_HISTORY_LENGTH 250

struct NoteHistoryEvent
{
   bool mOn;
   double mTime;
};

typedef list<NoteHistoryEvent> NoteHistoryList;

class NoteHistory
{
public:
   void AddEvent(double time, bool on);
   void Lock(string name) { mHistoryMutex.Lock(name); }
   void Unlock() { mHistoryMutex.Unlock(); }
   NoteHistoryList& GetHistory() { return mHistory; }
   bool CurrentlyOn();
private:
   NoteHistoryList mHistory;
   NamedMutex mHistoryMutex;
};

class INoteSource;

//intercepts notes to keep track of them for visualization
class NoteOutput : public INoteReceiver
{
public:
   NoteOutput(INoteSource* source) : mNoteSource(source) {}
   
   void Flush();
   void FlushTarget(INoteReceiver* target);
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;

   list<int> GetHeldNotes() { Poco::FastMutex::ScopedLock lock(mNotesMutex); return mNotes; }
   NoteHistory& GetNoteHistory() { return mNoteHistory; }
private:
   list<int> mNotes;
   ofMutex mNotesMutex;
   NoteHistory mNoteHistory;
   INoteSource* mNoteSource;
};

class INoteSource : public virtual IPatchable
{
public:
   INoteSource() : mIsNoteOrigin(false), mNoteOutput(this) {}
   virtual ~INoteSource() {}
   NoteOutput* GetNoteOutput() { return &mNoteOutput; }
   void PlayNoteOutput(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr);
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

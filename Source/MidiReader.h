//
//  MidiReader.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#ifndef __Bespoke__MidiReader__
#define __Bespoke__MidiReader__

#include "OpenFrameworksPort.h"

//TODO_PORT(Ryan) make this work

class MidiReader
{
public:
   MidiReader();
   virtual ~MidiReader();
   void Read(const char* midiFileName);
   float GetTempo(double ms);
   int GetBeat(double ms);
   void GetMeasurePos(double ms, int& measure, float& measurePos);
   void SetBeatOffset(int beatOffset) { mBeatOffset = beatOffset; }
   //MIDISequencer* GetSequencer() { return mSequencer; }
   
private:
   //MIDIMultiTrack mTracks;
   //MIDISequencer* mSequencer;
   int mBeatOffset;
};

#endif /* defined(__Bespoke__MidiReader__) */

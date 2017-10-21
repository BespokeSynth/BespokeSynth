//
//  MidiReader.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#include "MidiReader.h"

MidiReader::MidiReader()
: /*mSequencer(nullptr)
,*/ mBeatOffset(0)
{
   
}

MidiReader::~MidiReader()
{
   //delete mSequencer;
}

void MidiReader::Read(const char *midiFileName)
{
   /*MIDITimedBigMessage event;
   int eventTrack = 0;
   string filePath = ofToDataPath(midiFileName, true);
   
   MIDIFileReadStreamFile rs ( filePath.c_str() );
   
   if ( !rs.IsValid() )
   {
      ofLogError( "ERROR OPENING FILE AT: ",  filePath);
   }
   
   MIDIFileReadMultiTrack track_loader ( &mTracks );
   MIDIFileRead reader ( &rs, &track_loader );
   
   int numMidiTracks = reader.ReadNumTracks();
   int midiFormat = reader.GetFormat();
   
   mTracks.ClearAndResize( numMidiTracks );
   cout << "numMidiTracks: " << numMidiTracks << endl;
   cout << "midiFormat: " << midiFormat << endl;
   
   if ( reader.Parse() )
   {
      cout << "reader parsed!" << endl;
   }
   
   delete mSequencer;
   mSequencer = new MIDISequencer(&mTracks);*/
}

float MidiReader::GetTempo(double ms)
{
   /*if (mSequencer)
   {
      mSequencer->GoToTimeMs(ms);
      return mSequencer->GetCurrentTempo();
   }*/
   
   return 120;
}

int MidiReader::GetBeat(double ms)
{
   /*if (mSequencer)
   {
      mSequencer->GoToTimeMs(ms);
      return mSequencer->GetCurrentBeat();
   }*/
   
   return 0;
}

void MidiReader::GetMeasurePos(double ms, int& measure, float& measurePos)
{
   /*if (mSequencer)
   {
      mSequencer->GoToTimeMs(ms);
      
      int beat = (mSequencer->GetCurrentBeat() + mBeatOffset + 16) % 4;
      measure = mSequencer->GetCurrentMeasure() + ((mSequencer->GetCurrentBeat() + mBeatOffset + 16) / 4 - 4);
      
      double msIn = ms-mSequencer->GetCurrentTimeInMs();
      double bpm = mSequencer->GetCurrentTempo();
      double bpms = bpm / 60 / 1000;
      double beatProgress = bpms * msIn;
      measurePos = float((beat + beatProgress) / 4.0f);
   }
   else
   {
      measure = 0;
      measurePos = 0;
   }*/
}

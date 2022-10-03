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
//  MidiReader.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#include "MidiReader.h"

MidiReader::MidiReader()
//: mSequencer(nullptr)
{
}

MidiReader::~MidiReader()
{
   //delete mSequencer;
}

void MidiReader::Read(const char* midiFileName)
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

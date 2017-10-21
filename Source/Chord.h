//
//  Chord.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/13.
//
//

#ifndef __modularSynth__Chord__
#define __modularSynth__Chord__

#include <iostream>
#include "OpenFrameworksPort.h"

struct ScalePitches;

enum ChordType
{
   kChord_Maj = 0x1,
   kChord_Min = 0x2,
   kChord_Aug = 0x4,
   kChord_Dim = 0x8,
   kChord_Unknown = 0xff
};

struct Chord
{
   Chord() {}
   Chord(string name);
   Chord(int pitch, ChordType type, int inversion = 0) : mRootPitch(pitch), mType(type), mInversion(inversion) {}
   
   int mRootPitch;
   ChordType mType;
   int mInversion;
   
   string Name(bool withDegree, bool withAccidentals, ScalePitches* scale = nullptr);
   void SetFromDegreeAndScale(int degree, const ScalePitches& scale, int inversion = 0);
};


#endif /* defined(__modularSynth__Chord__) */

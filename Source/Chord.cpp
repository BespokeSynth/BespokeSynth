//
//  Chord.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/13.
//
//

#include "Chord.h"
#include "SynthGlobals.h"
#include "Scale.h"

Chord::Chord(string name)
{
   assert(name.length() >= 4);
   
   string pitchName;
   string typeName;
   if (name[1] == 'b' || name[1] == '#')
   {
      pitchName = name.substr(0,2);
      typeName = name.substr(2,name.length()-2);
   }
   else
   {
      pitchName = name.substr(0,1);
      typeName = name.substr(1,name.length()-1);
   }
   
   mRootPitch = PitchFromNoteName(pitchName);
   
   if (typeName == "maj")
      mType = kChord_Maj;
   else if (typeName == "min")
      mType = kChord_Min;
   else if (typeName == "aug")
      mType = kChord_Aug;
   else if (typeName == "dim")
      mType = kChord_Dim;
   else
      mType = kChord_Unknown;
}

string Chord::Name(bool withDegree, bool withAccidentals, ScalePitches* scale)
{
   if (scale == nullptr)
      scale = &TheScale->GetScalePitches();
   
   int degree;
   vector<Accidental> accidentals;
   if (withDegree || withAccidentals)
      scale->GetChordDegreeAndAccidentals(*this, degree, accidentals);
   
   string chordName = NoteName(mRootPitch);
   if (mType == kChord_Maj)
      chordName += "maj";
   else if (mType == kChord_Min)
      chordName += "min";
   else if (mType == kChord_Aug)
      chordName += "aug";
   else if (mType == kChord_Dim)
      chordName += "dim";
   else
      chordName += "???";
   
   if (mInversion != 0)
      chordName += "-"+ofToString(mInversion);
   
   if (withDegree)
      chordName += " "+GetRomanNumeralForDegree(degree)+" ";
   
   if (withAccidentals)
   {
      string accidentalList;
      for (int i=0; i<accidentals.size(); ++i)
         accidentalList += ofToString(accidentals[i].mPitch) + (accidentals[i].mDirection == 1? "#" : "b") + " ";
      chordName += accidentalList;
   }
   
   return chordName;
}

void Chord::SetFromDegreeAndScale(int degree, const ScalePitches& scale, int inversion /*= 0*/)
{
   mRootPitch = scale.GetPitchFromTone(degree) % TheScale->GetTet();
   mType = kChord_Unknown;
   if (scale.IsInScale(mRootPitch+4))
   {
      if (scale.IsInScale(mRootPitch+7))
         mType = kChord_Maj;
      else if (scale.IsInScale(mRootPitch+8))
         mType = kChord_Aug;
   }
   else if (scale.IsInScale(mRootPitch+3))
   {
      if (scale.IsInScale(mRootPitch+7))
         mType = kChord_Min;
      else if (scale.IsInScale(mRootPitch+6))
         mType = kChord_Dim;
   }
   mInversion = inversion;
}

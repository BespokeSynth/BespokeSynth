/*
  ==============================================================================

    ChordDatabase.cpp
    Created: 26 Mar 2018 9:54:44pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChordDatabase.h"
#include "Scale.h"

ChordDatabase::ChordDatabase()
{
   mChordShapes.push_back(ChordShape("maj", {0,4,7}));
   mChordShapes.push_back(ChordShape("min", {0,3,7}));
   mChordShapes.push_back(ChordShape("aug", {0,4,8}));
   mChordShapes.push_back(ChordShape("dim", {0,3,6}));
   mChordShapes.push_back(ChordShape("6", {0,4,7,9}));
   mChordShapes.push_back(ChordShape("7", {0,4,7,10}));
   mChordShapes.push_back(ChordShape("maj7", {0,4,7,11}));
   mChordShapes.push_back(ChordShape("min/maj7", {0,3,7,11}));
   mChordShapes.push_back(ChordShape("m7", {0,3,7,10}));
   mChordShapes.push_back(ChordShape("aug/maj7", {0,4,8,11}));
   mChordShapes.push_back(ChordShape("aug7", {0,4,8,10}));
   mChordShapes.push_back(ChordShape("min7dim5", {0,3,6,10}));
   mChordShapes.push_back(ChordShape("min7", {0,3,6,9}));
   mChordShapes.push_back(ChordShape("dom7dim5", {0,4,6,10}));
   mChordShapes.push_back(ChordShape("9", {0,4,7,10,14}));
   mChordShapes.push_back(ChordShape("11", {0,4,7,10,14,17}));
   mChordShapes.push_back(ChordShape("maj9", {0,4,7,11,14}));
   mChordShapes.push_back(ChordShape("maj11", {0,4,7,11,14,17}));
   mChordShapes.push_back(ChordShape("m9", {0,3,7,10,14}));
   mChordShapes.push_back(ChordShape("m6", {0,3,7,9}));
   mChordShapes.push_back(ChordShape("sus4", {0,5,7}));
   mChordShapes.push_back(ChordShape("sus2", {0,2,7}));
}

string ChordDatabase::GetChordName(vector<int> pitches) const
{
   int numPitches = (int)pitches.size();
   if (numPitches < 3)
      return "None";
   
   list<string> names;
   
   sort(pitches.begin(), pitches.end());
   for (int inversion = 0; inversion<numPitches; ++inversion)
   {
      for (ChordShape shape : mChordShapes)
      {
         if (shape.mElements.size() == numPitches)
         {
            int root = pitches[(numPitches-inversion)%numPitches] - (inversion > 0 ? 12 : 0);
            bool match = true;
            for (int i=0; i<numPitches; ++i)
            {
               if (shape.mElements[(i+inversion)%numPitches] != pitches[i] - root - (i >= numPitches - inversion ? 12 : 0))
               {
                  match = false;
                  break;
               }
            }
            if (match)
            {
               int degree = TheScale->GetToneFromPitch(root) % 7;
               names.push_back(NoteName(root) + shape.mName + (inversion == 0 ? "" : " (" + ofToString(inversion) + "inv)") + " (" + GetRomanNumeralForDegree(degree) + ")");
            }
         }
      }
   }
   
   if (names.size() == 0)
      return "Unknown";
   
   string ret = "";
   for (string name : names)
      ret += name + "; ";
   return ret.substr(0, ret.length()-2);
}

vector<int> ChordDatabase::GetChord(string name, int inversion) const
{
   vector<int> ret;
   for (auto shape : mChordShapes)
   {
      if (shape.mName == name)
      {
         bool isInverted = (inversion != 0);
         for (int i=0; i<shape.mElements.size(); ++i)
         {
            int index = (i + inversion) % shape.mElements.size();
            int val = shape.mElements[index];
            if (index >= inversion && isInverted)
               val -= 12;
            ret.push_back(val);
         }
      }
   }
   return ret;
}

vector<string> ChordDatabase::GetChordNames() const
{
   vector<string> ret;
   
   for (auto shape : mChordShapes)
      ret.push_back(shape.mName);
   
   return ret;
}

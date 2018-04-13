/*
  ==============================================================================

    ChordDatabase.h
    Created: 26 Mar 2018 9:54:44pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "SynthGlobals.h"

class ChordDatabase
{
public:
   ChordDatabase();
   string GetChordName(vector<int> pitches) const;
   vector<int> GetChord(string name, int inversion) const;
   vector<string> GetChordNames() const;
private:
   struct ChordShape
   {
      ChordShape(string name, vector<int> elements)
      {
         mName = name;
         mElements = elements;
      }
      string mName;
      vector<int> mElements;
   };
   vector<ChordShape> mChordShapes;
};

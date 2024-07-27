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
/*
  ==============================================================================

    ChordDatabase.h
    Created: 26 Mar 2018 9:54:44pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <numeric>
#include <set>
#include <string>
#include <vector>

class ChordDatabase
{
public:
   ChordDatabase();
   std::string GetChordName(std::vector<int> pitches) const;
   std::set<std::string> GetChordNamesAdvanced(const std::vector<int>& pitches, bool useScaleDegrees, bool showIntervals) const;
   std::vector<int> GetChord(std::string name, int inversion) const;
   std::vector<std::string> GetChordNames() const;

private:
   struct ChordShape
   {
      ChordShape(std::string name, std::vector<int> elements, float rootPosBias = 0.0f)
      {
         std::vector<float> weights(12);
         mName = name;
         mElements = elements;
         mWeights = weights;
         mWeightSum = 0;
         mRootPosBias = rootPosBias;
      }

      ChordShape(std::string name, std::vector<int> elements, std::vector<float> weights, float rootPosBias = 0.0f)
      {
         mName = name;
         mElements = elements;
         mWeights = weights;
         auto lambda = [&](float a, float b)
         {
            return b > 0.0f ? a - b : a;
         };
         mWeightSum = std::accumulate(
         mWeights.begin(), mWeights.end(), 0.0f, lambda);
         mRootPosBias = rootPosBias;
      }
      std::string mName;
      std::vector<int> mElements;
      std::vector<float> mWeights;
      float mWeightSum;
      float mRootPosBias;
   };
   std::vector<ChordShape> mChordShapes;

   std::string GetChordNameAdvanced(const std::vector<int>& pitches, const int root, const ChordShape shape, bool useScaleDegrees) const;
   std::string NoteNameScaleRelative(int pitch, bool useDegrees) const; // Helper function for GetChordNameAdvanced, may be better off moved to synthglobals?
   std::string ChordNameScaleRelative(int rootPitch) const; // Helper function for GetChordNameAdvanced, may be better off moved to synthglobals?
};

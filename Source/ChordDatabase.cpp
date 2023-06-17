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

    ChordDatabase.cpp
    Created: 26 Mar 2018 9:54:44pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChordDatabase.h"
#include "Scale.h"
#include <set>

ChordDatabase::ChordDatabase()
{
   // Major scale like chords
   //
   // { 10.0f, -2.0f, -1.0f, 10.0f, -2.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -1.0f } // Based on Mixolydian/Ionian mode

   // ref:                           {  0/12,  1/13,  2/14,  3/15,  4/16,  5/17,  6/18,  7/19,  8/20,  9/21, 10/22, 11/23}));
   //                                {     C,    C#,     D,    D#,     E,     F,    F#,     G,    G#,     A,    A#,     B}));

   mChordShapes.push_back(ChordShape("", { 0, 4, 7 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -1.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("sus4", { 0, 5, 7 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, -5.0f, 10.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("sus2", { 0, 2, 7 },
                                     { 10.0f, -2.0f, 10.0f, -5.0f, -5.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f }, 2.0f));

   mChordShapes.push_back(ChordShape("2", { 0, 2, 4, 7 },
                                     { 10.0f, -2.0f, 10.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("4", { 0, 4, 5, 7 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, 10.0f, 10.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("6", { 0, 4, 7, 9 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, 10.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("7", { 0, 4, 7, 10 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, 10.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("9", { 0, 4, 7, 10, 14 },
                                     { 10.0f, -2.0f, 10.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, 8.00f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("11", { 0, 4, 7, 10, 14, 17 },
                                     { 10.0f, -2.0f, 8.00f, -5.0f, 10.0f, 10.0f, -2.0f, 10.0f, -2.0f, -1.0f, 8.00f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("13", { 0, 4, 7, 10, 14, 17, 21 },
                                     { 10.0f, -2.0f, 8.00f, -2.0f, 10.0f, 8.00f, -2.0f, 10.0f, -2.0f, 10.0f, 8.00f, -2.0f }, 2.0f));


   mChordShapes.push_back(ChordShape("6/9", { 0, 4, 7, 9, 14 },
                                     { 10.0f, -2.0f, 10.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, 10.0f, -5.0f, -5.0f }, 2.0f));


   mChordShapes.push_back(ChordShape("maj7", { 0, 4, 7, 11 },
                                     { 10.0f, -2.0f, -1.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -2.0f, 10.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("maj9", { 0, 4, 7, 11, 14 },
                                     { 10.0f, -2.0f, 10.0f, -5.0f, 10.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -2.0f, 8.00f }, 2.0f));
   mChordShapes.push_back(ChordShape("maj11", { 0, 4, 7, 11, 14, 17 },
                                     { 10.0f, -2.0f, 8.00f, -5.0f, 10.0f, 10.0f, -2.0f, 10.0f, -2.0f, -1.0f, -2.0f, 8.00f }, 2.0f));
   mChordShapes.push_back(ChordShape("maj13", { 0, 4, 7, 11, 14, 17, 21 },
                                     { 10.0f, -2.0f, 8.00f, -5.0f, 10.0f, 8.00f, -2.0f, 10.0f, -2.0f, 10.0f, -2.0f, 8.00f }, 2.0f));

   // Minor scale like chords
   // { 10.0f, -2.0f, -1.0f, 10.0f, -2.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -2.0f, -1.0f } // Based on Dorian/Aeolian mode

   // ref:                           {  0/12,  1/13,  2/14,  3/15,  4/16,  5/17,  6/18,  7/19,  8/20,  9/21, 10/22, 11/23}));
   //                                {     C,    C#,     D,    D#,     E,     F,    F#,     G,    G#,     A,    A#,     B}));

   mChordShapes.push_back(ChordShape("m", { 0, 3, 7 },
                                     { 10.0f, -2.0f, -1.0f, 10.0f, -5.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("m6", { 0, 3, 7, 9 },
                                     { 10.0f, -2.0f, -1.0f, 10.0f, -5.0f, -1.0f, -2.0f, 10.0f, -2.0f, 10.0f, -1.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("m7", { 0, 3, 7, 10 },
                                     { 10.0f, -2.0f, -1.0f, 10.0f, -5.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, 10.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("m9", { 0, 3, 7, 10, 14 },
                                     { 10.0f, -2.0f, 10.0f, 10.0f, -5.0f, -1.0f, -2.0f, 10.0f, -2.0f, -1.0f, 8.00f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("m11", { 0, 3, 7, 10, 14, 17 },
                                     { 10.0f, -2.0f, 8.00f, 10.0f, -5.0f, 10.0f, -2.0f, 10.0f, -2.0f, -1.0f, 8.00f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("m13", { 0, 3, 7, 10, 14, 17, 21 },
                                     { 10.0f, -2.0f, 8.00f, 10.0f, -5.0f, 8.00f, -2.0f, 10.0f, -2.0f, 10.0f, 8.00f, -2.0f }, 2.0f));

   // Mixed
   // { 10.0f, -2.0f, -2.0f, -1.0f, 10.0f, -2.0f, -2.0f, -1.0f, 10.0f, -2.0f, -2.0f, -1.0f } // Based on augmented scale
   // { 10.0f, -2.0f, -1.0f, 10.0f, -2.0f, -1.0f, 10.0f, -2.0f, -1.0f, -1.0f, -2.0f, -1.0f } // Based on diminished scale
   // { 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f } // no scale? {:^c (i.e. don't alter mixed chords)

   // ref:                           {  0/12,  1/13,  2/14,  3/15,  4/16,  5/17,  6/18,  7/19,  8/20,  9/21, 10/22, 11/23}));
   //                                {     C,    C#,     D,    D#,     E,     F,    F#,     G,    G#,     A,    A#,     B}));
   mChordShapes.push_back(ChordShape("aug", { 0, 4, 8 },
                                     { 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("aug7", { 0, 4, 8, 10 },
                                     { 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, 10.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("aug/maj7", { 0, 4, 8, 11 },
                                     { 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f }, 2.0f));

   mChordShapes.push_back(ChordShape("dim", { 0, 3, 6 },
                                     { 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("dim7", { 0, 3, 6, 9 },
                                     { 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("min7dim5", { 0, 3, 6, 10 },
                                     { 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f }, 2.0f));

   mChordShapes.push_back(ChordShape("dom7dim5", { 0, 4, 6, 10 },
                                     { 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, 10.0f, -2.0f }, 2.0f));
   mChordShapes.push_back(ChordShape("min/maj7", { 0, 3, 7, 11 },
                                     { 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, 10.0f, -2.0f, -2.0f, -2.0f, -2.0f, 10.0f }, 2.0f));
}

std::set<std::string> ChordDatabase::GetChordNamesAdvanced(const std::vector<int>& pitches, bool useScaleDegrees, bool showIntervals) const
{
   std::set<std::string> chordNames;

   int numPitches = (int)pitches.size();
   if (numPitches == 1 && useScaleDegrees && showIntervals)
   {
      // Show scale degree of played note. Not really chord detection, but may be useful
      chordNames.insert(NoteNameScaleRelative(pitches[0], true));
      return chordNames;
   }
   else if (numPitches == 2 && showIntervals)
   {
      // Intervals up to 11th
      const std::vector<std::string> intervals = { "1st", "min 2nd", "2nd", "min 3rd", "3rd", "4th", "aug 4th/dim 5th", "5th",
                                                   "min 6th", "6th", "min 7th", "7th", "oct", "min 9th", "9th", "min 10th", "10th", "11th" };
      int interval = abs(pitches[1] - pitches[0]);
      int lowest = (pitches[0] < pitches[1] ? pitches[0] : pitches[1]) % 12;
      if (interval < intervals.size())
         chordNames.insert(intervals[interval] + "/" + NoteNameScaleRelative(lowest, useScaleDegrees));

      return chordNames;
   }

   // Create a boolean vector with each pitch played, set to be in one octave
   std::set<int> octavePitches;

   for (int pitch : pitches)
   {
      octavePitches.insert(pitch % 12);
   }

   if (octavePitches.size() < 3)
      return chordNames;

   // Considering each played note as a possible root, find the root and chord with the greatest weight

   float maxWeight = 0.0f;
   int lowestPitch = pitches[0] % 12;

   std::list<std::tuple<int, ChordShape>> bestChords; // Is this cursed?

   // For each note played
   for (int rootOctavePitch : octavePitches)
   {
      // Try note as the root, multiply with the weights of the notes to be played
      for (ChordShape shape : mChordShapes)
      {
         float chordWeight = shape.mWeightSum;

         // Add some extra weight if the lowest played note is the root
         chordWeight += rootOctavePitch == lowestPitch ? shape.mRootPosBias : 0;

         // Add the weights for the pitches in the chord
         for (int octavePitch : octavePitches)
         {
            // Looping over the same stuff within the same loop, crazy!
            chordWeight += 2.0f * shape.mWeights[(12 + octavePitch - rootOctavePitch) % 12];
         }

         // Consider the chords with the highest weight as the best fit
         if (chordWeight > maxWeight + FLT_EPSILON)
         {
            maxWeight = chordWeight;

            // Better weight than found before, replace list with this chord
            bestChords.clear();
            bestChords.push_back(std::make_tuple(rootOctavePitch, shape));
         }
         else if (chordWeight >= maxWeight - FLT_EPSILON)
         {
            // Equal weight as current best, add to list
            bestChords.push_back(std::make_tuple(rootOctavePitch, shape));
         }
      }
   }

   for (const auto& chord : bestChords)
   {
      chordNames.insert(GetChordNameAdvanced(pitches, std::get<0>(chord), std::get<1>(chord), useScaleDegrees));
   }

   return chordNames;
}

std::string ChordDatabase::GetChordNameAdvanced(const std::vector<int>& pitches, const int root, const ChordShape shape, bool useScaleDegrees) const
{
   std::string rootName;
   std::string chordName;
   if (useScaleDegrees)
   {
      rootName = ChordNameScaleRelative(root);
      chordName = rootName + shape.mName;
   }
   else
   {
      rootName = NoteNameScaleRelative(root, false);
      chordName = rootName + shape.mName;
   }


   // Alterations

   std::set<int> rootScalePitches;
   const std::set<int> majorScalePitches = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23 };
   const std::vector<std::string> Alterations = { "1", "(m2)", "2", "(m3)", "3", "4", "(b5)", "5", "(#5)", "6", "(b7)", "7",
                                                  "8", "(b9)", "9", "(#9)", "10", "11", "(#11)", "12", "(b13)", "13", "(#13)", "14" };

   std::string alterations = "";

   int rootPitch = 0;
   bool foundRoot = false;

   // Find hightest root before other notes (Needed to distinguish between notes in different octaves and for inverted chords)
   for (int pitch : pitches)
   {
      if ((12 + pitch - root) % 12 == 0)
      {
         rootPitch = pitch;
         foundRoot = true;
         continue;
      }
      if (foundRoot)
         break;
   }

   // Calculate pitches in the key of the root
   for (int pitch : pitches)
   {
      // Use difference between highest root up to two octaves, otherwise use first octave
      rootScalePitches.insert(pitch - rootPitch >= 0 ? (pitch - rootPitch) % 24 : (12 + pitch - root) % 12);
   }

   // For all played notes
   for (int shapePitch : shape.mElements)
   {
      // If pitch played, continue
      if (rootScalePitches.find(shapePitch) != rootScalePitches.end() ||
          rootScalePitches.find((shapePitch + 12) % 24) != rootScalePitches.end())
      {
         rootScalePitches.erase(shapePitch);
         rootScalePitches.erase((shapePitch + 12) % 24);
         continue;
      }

      // If pitch not played, test if alteration played instead
      // shapepitch has to be in scale of root and alteration has to be played
      if (majorScalePitches.find(shapePitch) != majorScalePitches.end())
      {
         // Don't need to modulo the indices here because rootScalePitches are <24
         // and the indices must be found in rootScalePitches

         // Alterations up
         if (rootScalePitches.find(shapePitch + 1) != rootScalePitches.end())
         {
            alterations += Alterations[shapePitch + 1];
            rootScalePitches.erase((shapePitch + 1));
         }
         else if (rootScalePitches.find(shapePitch + 12 + 1) != rootScalePitches.end())
         {
            // No +12 as the note from the shape is altered, not the note found in rootScalePitches
            alterations += Alterations[shapePitch + 1];
            rootScalePitches.erase((shapePitch + 12 + 1));
         }
         // Alterations down
         else if (rootScalePitches.find(shapePitch - 1) != rootScalePitches.end())
         {
            alterations += Alterations[shapePitch - 1];
            rootScalePitches.erase((shapePitch - 1));
         }
         else if (rootScalePitches.find(shapePitch + 12 - 1) != rootScalePitches.end())
         {
            alterations += Alterations[shapePitch - 1];
            rootScalePitches.erase((shapePitch + 12 - 1));
         }
         // Otherwise, display missing pitches (except 5 because of neutral tone) as omitX
         else if (shapePitch != 7 && shapePitch != 19)
         {
            alterations += "omit" + Alterations[shapePitch];
         }
      }
   }

   // Display left-over pitches as addX
   for (int playedPitch : rootScalePitches)
   {
      alterations += "add" + Alterations[playedPitch];
   }

   // If lowest note is not the root, notate it ass the bass note
   // (kind of extra since there's lots of inversions, might need a check to see if note is in the chord shape, or a check on the distance between the rest of the notes)
   int lowest = 12 + pitches[0] - root % 12;
   if (lowest % 12 != 0)
   {
      alterations += "/" + NoteNameScaleRelative(pitches[0], useScaleDegrees);
   }

   return chordName + alterations;
}

std::string ChordDatabase::ChordNameScaleRelative(int rootPitch) const
{
   const std::vector<std::string> chordNames = { "I", "bII", "II", "bIII", "III", "IV", "bV", "V", "bVI", "VI", "bVII", "VII" };
   int relpitch = (rootPitch + 12 - TheScale->ScaleRoot()) % 12;
   return chordNames[relpitch];
}

std::string ChordDatabase::NoteNameScaleRelative(int pitch, bool useDegrees) const
{

   if (useDegrees)
   {
      int relpitch = (pitch + 12 - TheScale->ScaleRoot()) % 12;

      //const std::vector<std::string> flats = { "1^", "2^b", "2^", "3^b", "3^", "4^", "5^b", "5^", "6^b", "6^", "7^b", "7^" };
      //const std::vector<std::string> sharps = { "1^", "1^#", "2^", "2^#", "3^", "4^", "4^#", "5^", "5^#", "6^", "6^#", "7^" };
      // Choice of flats or sharps here depends strongly on context that can't really be gathered, so instead use some common options
      const std::vector<std::string> accidentals = { "1^", "2^b", "2^", "3^b", "3^", "4^", "5^b", "5^", "5^#", "6^", "7^b", "7^" };

      // For consistency with scale types, all scales are related to the major scale. i.e. in C minor Cb is 3^b rather than 3^.
      return accidentals[relpitch];
   }
   else
   {
      pitch %= 12;

      if (TheScale->GetType() == "aeolian")
      {
         const std::vector<int> flatScales = { 0, 2, 3, 5, 8, 10 }; // D G C F Ab Eb Bb
         if (std::find(flatScales.begin(), flatScales.end(), TheScale->ScaleRoot()) != flatScales.end())
            return NoteName(pitch, true);
         else
            return NoteName(pitch);
      }
      else
      {
         const std::vector<int> flatScales = { 0, 1, 3, 5, 6, 8, 10 }; // C F Bb Eb Ab Gb Db (Cb not included)
         if (std::find(flatScales.begin(), flatScales.end(), TheScale->ScaleRoot()) != flatScales.end())
            return NoteName(pitch, true);
         else
            return NoteName(pitch);
      }
   }
}

std::string ChordDatabase::GetChordName(std::vector<int> pitches) const
{
   int numPitches = (int)pitches.size();
   if (numPitches < 3)
      return "None";

   std::list<std::string> names;

   sort(pitches.begin(), pitches.end());
   for (int inversion = 0; inversion < numPitches; ++inversion)
   {
      for (ChordShape shape : mChordShapes)
      {
         if (shape.mElements.size() == numPitches)
         {
            int root = pitches[(numPitches - inversion) % numPitches] - (inversion > 0 ? 12 : 0);
            bool match = true;
            for (int i = 0; i < numPitches; ++i)
            {
               if (shape.mElements[(i + inversion) % numPitches] != pitches[i] - root - (i >= numPitches - inversion ? 12 : 0))
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

   std::string ret = "";
   for (std::string name : names)
      ret += name + "; ";
   return ret.substr(0, ret.length() - 2);
}

std::vector<int> ChordDatabase::GetChord(std::string name, int inversion) const
{
   std::vector<int> ret;
   for (auto shape : mChordShapes)
   {
      if (shape.mName == name)
      {
         bool isInverted = (inversion != 0);
         for (int i = 0; i < shape.mElements.size(); ++i)
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

std::vector<std::string> ChordDatabase::GetChordNames() const
{
   std::vector<std::string> ret;

   for (auto shape : mChordShapes)
      ret.push_back(shape.mName);

   return ret;
}

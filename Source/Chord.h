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
//  Chord.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/13.
//
//

#pragma once

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
   Chord(std::string name);
   Chord(int pitch, ChordType type, int inversion = 0)
   : mRootPitch(pitch)
   , mType(type)
   , mInversion(inversion)
   {}

   int mRootPitch{ 0 };
   ChordType mType{ kChord_Unknown };
   int mInversion{ 0 };

   std::string Name(bool withDegree, bool withAccidentals, ScalePitches* scale = nullptr);
   void SetFromDegreeAndScale(int degree, const ScalePitches& scale, int inversion = 0);
};

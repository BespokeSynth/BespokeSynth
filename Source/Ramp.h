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
//  Ramp.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/26/12.
//
//

#ifndef __modularSynth__Ramp__
#define __modularSynth__Ramp__

#include <iostream>
#include <array>

class Ramp
{
public:
   void Start(double curTime, float end, double endTime);
   void Start(double curTime, float start, float end, double endTime);
   void SetValue(float val);
   bool HasValue(double time) const;
   float Value(double time) const;
   float Target(double time) const { return GetCurrentRampData(time)->mEndValue; }

private:
   struct RampData
   {
      double mStartTime{ -1 };
      float mStartValue{ 0 };
      float mEndValue{ 1 };
      double mEndTime{ -1 };
   };

   const RampData* GetCurrentRampData(double time) const;

   std::array<RampData, 10> mRampDatas;
   int mRampDataPointer{ 0 };
};

#endif /* defined(__modularSynth__Ramp__) */

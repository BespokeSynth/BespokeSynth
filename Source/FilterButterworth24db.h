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
//  FilterButterworth24db.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#pragma once

class CFilterButterworth24db
{
public:
   CFilterButterworth24db(void);
   ~CFilterButterworth24db(void);
   void SetSampleRate(float fs);
   void Set(float cutoff, float q);
   void CopyCoeffFrom(CFilterButterworth24db& other);
   float Run(float input);
   void Clear();

private:
   float t0{ 0 }, t1{ 0 }, t2{ 0 }, t3{ 0 };
   float coef0{ 0 }, coef1{ 0 }, coef2{ 0 }, coef3{ 0 };
   float history1{ 0 }, history2{ 0 }, history3{ 0 }, history4{ 0 };
   float gain{ 0 };
   float min_cutoff{ 0 }, max_cutoff{ 0 };
};

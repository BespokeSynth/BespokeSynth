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

#ifndef __Bespoke__FilterButterworth24db__
#define __Bespoke__FilterButterworth24db__

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
   float t0, t1, t2, t3;
   float coef0, coef1, coef2, coef3;
   float history1, history2, history3, history4;
   float gain;
   float min_cutoff, max_cutoff;
};

#endif /* defined(__Bespoke__FilterButterworth24db__) */

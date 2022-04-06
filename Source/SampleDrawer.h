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
//  SampleDrawer.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#ifndef __Bespoke__SampleDrawer__
#define __Bespoke__SampleDrawer__

#include "OpenFrameworksPort.h"

class Sample;

class SampleDrawer
{
public:
   SampleDrawer()
   : mSample(nullptr)
   , mStartSample(0)
   , mEndSample(0)
   , mX(0)
   , mY(0)
   , mWidth(1)
   , mHeight(1) {}
   void SetSample(Sample* sample) { mSample = sample; }
   void SetPosition(float x, float y)
   {
      mX = x;
      mY = y;
   }
   void SetDimensions(float w, float h)
   {
      mWidth = w;
      mHeight = h;
   }
   void SetRange(int startSample, int endSample)
   {
      mStartSample = startSample;
      mEndSample = endSample;
   }
   void Draw(int playPosition = -1, float vol = 1, ofColor color = ofColor::black);
   void DrawLine(int sample, ofColor color);
   int GetSampleAtMouse(int x, int y);

private:
   Sample* mSample;
   int mStartSample;
   int mEndSample;
   float mX;
   float mY;
   float mWidth;
   float mHeight;
};

#endif /* defined(__Bespoke__SampleDrawer__) */

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
//  SampleDrawer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#include "SampleDrawer.h"
#include "Sample.h"
#include "SynthGlobals.h"

void SampleDrawer::Draw(int playPosition, float vol, ofColor color)
{
   ofPushMatrix();
   ofTranslate(mX, mY);
   mSample->LockDataMutex(true);
   DrawAudioBuffer(mWidth, mHeight, mSample->Data(), mStartSample, mEndSample, playPosition, vol, color);
   mSample->LockDataMutex(false);
   ofPopMatrix();
}

void SampleDrawer::DrawLine(int sample, ofColor color)
{
   ofPushStyle();
   ofSetColor(color);
   int position = ofMap(sample, mStartSample, mEndSample, mX, mX + mWidth, K(clamp));
   ofLine(position, mY, position, mY + mHeight);
   ofPopStyle();
}

int SampleDrawer::GetSampleAtMouse(int x, int y)
{
   if (x >= mX && y >= mY && x <= mX + mWidth && y <= mY + mHeight)
      return (int)ofMap(x, mX, mX + mWidth, mStartSample, mEndSample);
   return -1;
}

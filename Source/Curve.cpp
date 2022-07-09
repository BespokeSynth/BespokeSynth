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
//  Curve.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#include "Curve.h"
#include "FileStream.h"
#include <algorithm>

Curve::Curve()
: mNumCurvePoints(0)
, mStart(0)
, mEnd(1)
, mColor(ofColor::white)
, mLastEvalIndex(0)
{
}

void Curve::AddPoint(CurvePoint point)
{
   if (IsAtCapacity())
      return;

   bool inserted = false;
   for (int i = 0; i < mNumCurvePoints; ++i)
   {
      if (mPoints[i].mTime > point.mTime)
      {
         for (int j = i; j < mNumCurvePoints; ++j)
            mPoints[j + 1] = mPoints[j];
         mPoints[i] = point;
         inserted = true;
         ++mNumCurvePoints;
         break;
      }
   }
   if (!inserted)
      AddPointAtEnd(point);
}

void Curve::AddPointAtEnd(CurvePoint point)
{
   if (IsAtCapacity())
      return;
   mPoints[mNumCurvePoints] = point;
   ++mNumCurvePoints;
}

int Curve::FindIndexForTime(float time)
{
   int max = mNumCurvePoints - 1;
   int left = 0;
   int right = max;
   while (left <= right)
   {
      int mid = left + (right - left) / 2;

      if (mPoints[mid].mTime < time && (mid == max || mPoints[mid + 1].mTime >= time)) // Check if x is present at mid
         return mid;
      if (mPoints[mid].mTime < time) // If time greater, ignore left half
         left = mid + 1;
      else // If time is smaller, ignore right half
         right = mid - 1;
   }

   // if we reach here, then element was not present
   return -1;
}

float Curve::Evaluate(float time, bool holdEndForLoop)
{
   float retVal = 0;

   if (mNumCurvePoints > 0)
   {
      if (time <= mPoints[0].mTime)
      {
         if (holdEndForLoop)
            return mPoints[mNumCurvePoints - 1].mValue;
         else
            return mPoints[0].mValue;
      }

      int beforeIndex = 0;
      int quickCheckIndex = mLastEvalIndex;
      if (quickCheckIndex < mNumCurvePoints &&
          mPoints[quickCheckIndex].mTime < time &&
          (quickCheckIndex == mNumCurvePoints - 1 || mPoints[quickCheckIndex + 1].mTime >= time))
      {
         beforeIndex = quickCheckIndex;
      }
      else
      {
         /*for (int i=1; i<mNumCurvePoints; ++i)
         {
            if (mPoints[i].mTime >= time)
            {
               beforeIndex = i-1;
               break;
            }
         }*/
         beforeIndex = FindIndexForTime(time);
         assert(beforeIndex >= 0 && beforeIndex < mNumCurvePoints);
      }

      mLastEvalIndex = beforeIndex;
      int afterIndex = MIN(beforeIndex + 1, mNumCurvePoints - 1);

      retVal = ofMap(time, mPoints[beforeIndex].mTime, mPoints[afterIndex].mTime, mPoints[beforeIndex].mValue, mPoints[afterIndex].mValue, K(clamp));
   }

   return retVal;
}

void Curve::Render()
{
   ofPushStyle();
   ofNoFill();
   ofSetColor(mColor);
   ofBeginShape();
   for (int i = 0; i < mWidth; ++i)
   {
      float val = Evaluate(ofMap(float(i) / mWidth, 0, 1, mStart, mEnd));

      if (i > 0)
      {
         ofVertex(i + mX, mY + (1 - val) * mHeight);
      }
   }
   ofEndShape();
   ofPopStyle();
}

void Curve::Clear()
{
   mNumCurvePoints = 0;
}

CurvePoint* Curve::GetPoint(int index)
{
   assert(index < mNumCurvePoints);
   return &mPoints[index];
}

void Curve::OnClicked(float x, float y, bool right)
{
   ofLog() << "curve clicked";
}

bool Curve::MouseMoved(float x, float y)
{
   ofLog() << "curve mousemoved";
   return false;
}

bool Curve::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   ofLog() << "curve mousescrolled";
   return false;
}

namespace
{
   const int kSaveStateRev = 1;
}

void Curve::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mNumCurvePoints;
   for (int i = 0; i < mNumCurvePoints; ++i)
      out << mPoints[i].mTime << mPoints[i].mValue;
}

void Curve::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   in >> mNumCurvePoints;
   for (int i = 0; i < mNumCurvePoints; ++i)
      in >> mPoints[i].mTime >> mPoints[i].mValue;
}

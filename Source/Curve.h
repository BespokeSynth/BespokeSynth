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
//  Curve.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#pragma once

#include "OpenFrameworksPort.h"
#include "IClickable.h"

class FileStreamOut;
class FileStreamIn;

struct CurvePoint
{
public:
   CurvePoint() {}
   CurvePoint(double time, double value)
   : mTime(time)
   , mValue(value)
   {}
   double mTime{ 0 };
   double mValue{ 0 };
};

class Curve : public IClickable
{
public:
   Curve(double defaultValue);
   void AddPoint(CurvePoint point);
   void AddPointAtEnd(CurvePoint point); //only use this if you are sure that there are no points already added at an earlier time
   double Evaluate(double time, bool holdEndForLoop = false);
   void Render() override;
   void SetExtents(double start, double end)
   {
      mStart = start;
      mEnd = end;
   }
   void SetColor(ofColor color) { mColor = color; }
   void GetDimensions(double& width, double& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void SetDimensions(float width, float height)
   {
      mWidth = width;
      mHeight = height;
   }
   void Clear();
   int GetNumPoints() const { return mNumCurvePoints; }
   CurvePoint* GetPoint(int index);

   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

protected:
   void OnClicked(double x, double y, bool right) override;
   bool MouseMoved(double x, double y) override;
   bool MouseScrolled(double x, double y, double scrollX, double scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

private:
   bool IsAtCapacity() { return mNumCurvePoints >= (int)mPoints.size(); }
   int FindIndexForTime(double time);
   std::array<CurvePoint, 5000> mPoints;
   int mNumCurvePoints{ 0 };
   double mWidth{ 200 };
   double mHeight{ 20 };
   double mStart{ 0 };
   double mEnd{ 1 };
   ofColor mColor{ ofColor::white };
   int mLastEvalIndex{ 0 };
   double mDefaultValue{ 0 };
};

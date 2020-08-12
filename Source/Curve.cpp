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
{
}

void Curve::AddPoint(CurvePoint point)
{
   if (IsAtCapacity())
      return;
   
   bool inserted = false;
   for (int i=0; i<mNumCurvePoints; ++i)
   {
      if (mPoints[i].mTime > point.mTime)
      {
         for (int j=i; j<mNumCurvePoints; ++j)
            mPoints[j+1] = mPoints[j];
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

float Curve::Evaluate(float time, bool holdEndForLoop)
{
   float retVal = 0;
   
   if (mNumCurvePoints > 0)
   {
      if (holdEndForLoop && time < mPoints[0].mTime)
         return mPoints[mNumCurvePoints-1].mValue;
      
      const CurvePoint* before = &mPoints[0];
      const CurvePoint* after = nullptr;
      for (int i=0; i<mNumCurvePoints; ++i)
      {
         if (mPoints[i].mTime < time)
         {
            before = &mPoints[i];
         }
         else
         {
            after = &mPoints[i];
            break;
         }
      }
      if (after == nullptr)
         after = before;
      
      retVal = ofMap(time, before->mTime, after->mTime, before->mValue, after->mValue, K(clamp));
   }
   
   return retVal;
}

void Curve::Render()
{
   ofPushStyle();
   ofNoFill();
   ofSetColor(mColor);
   ofBeginShape();
   for (int i=0; i<mWidth; ++i)
   {
      float val = Evaluate(ofMap(float(i)/mWidth,0,1,mStart,mEnd));
      
      if (i > 0)
      {
         ofVertex(i+mX,mY+(1-val)*mHeight);
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

void Curve::OnClicked(int x, int y, bool right)
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
   for (int i=0; i<mNumCurvePoints; ++i)
      out << mPoints[i].mTime << mPoints[i].mValue;
}

void Curve::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   in >> mNumCurvePoints;
   for (int i=0; i<mNumCurvePoints; ++i)
      in >> mPoints[i].mTime >> mPoints[i].mValue;
}

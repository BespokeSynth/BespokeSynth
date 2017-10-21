//
//  Curve.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#include "Curve.h"

Curve::Curve()
: mStart(0)
, mEnd(1)
, mColor(ofColor::white)
{
}

void Curve::AddPoint(CurvePoint point)
{
   mCurveMutex.lock();
   bool inserted = false;
   for (auto iter = mPoints.begin(); iter != mPoints.end(); ++iter)
   {
      if (iter->mTime > point.mTime)
      {
         mPoints.insert(iter, point);
         inserted = true;
         break;
      }
   }
   if (!inserted)
      mPoints.push_back(point);
   mCurveMutex.unlock();
}

void Curve::AddPointAtEnd(CurvePoint point)
{
   mCurveMutex.lock();
   mPoints.push_back(point);
   mCurveMutex.unlock();
}

float Curve::Evaluate(float time)
{
   float retVal = 0;
   
   mCurveMutex.lock();
   
   if (mPoints.empty() == false)
   {
      const CurvePoint* before = &(*mPoints.begin());
      const CurvePoint* after = nullptr;
      for (const auto& point : mPoints)
      {
         if (point.mTime < time)
         {
            before = &point;
         }
         else
         {
            after = &point;
            break;
         }
      }
      if (after == nullptr)
         after = before;
      
      retVal = ofMap(time, before->mTime, after->mTime, before->mValue, after->mValue);
   }
   mCurveMutex.unlock();
   
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

void Curve::DeleteBetween(float start, float end)
{
   mCurveMutex.lock();
   
   //ofLog() << mPoints.size() << ": delete between " << start << " " << end;
   
   if (mPoints.empty() == false)
   {
      list<CurvePoint>::iterator before = mPoints.end();
      list<CurvePoint>::iterator after = mPoints.begin();
      int i=0;
      int ibefore = -1;
      int iafter = -1;
      bool lookingForStart = true;
      for (auto iter = mPoints.begin(); iter != mPoints.end(); ++iter)
      {
         //ofLog() << i << ": " << iter->mTime;
         if (iter->mTime > start && lookingForStart)
         {
            before = iter;
            ibefore = i;
            lookingForStart = false;
         }
         else if (iter->mTime < end && !lookingForStart)
         {
            after = iter;
            iafter = i;
         }
         
         ++i;
      }
      
      //ofLog() << ibefore << " " << iafter;
      
      if (before != mPoints.end() && after != mPoints.begin())
      {
         if (start < end)
         {
            mPoints.erase(before, after);
         }
         else
         {
            mPoints.erase(before, mPoints.end());
            
            list<CurvePoint>::iterator after = mPoints.end();
            for (auto iter = mPoints.begin(); iter != mPoints.end(); ++iter)
            {
               if (iter->mTime < end)
                  after = iter;
            }
            
            mPoints.erase(mPoints.begin(), after);
         }
      }
   }
   mCurveMutex.unlock();
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

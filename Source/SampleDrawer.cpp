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
   ofTranslate(mX,mY);
   mSample->LockDataMutex(true);
   DrawAudioBuffer(mWidth, mHeight, mSample->Data(), mStartSample, mEndSample, playPosition, vol, color);
   mSample->LockDataMutex(false);
   ofPopMatrix();
}

void SampleDrawer::DrawLine(int sample, ofColor color)
{
   ofPushStyle();
   ofSetColor(color);
   int position =  ofMap(sample, mStartSample, mEndSample, mX, mX+mWidth, K(clamp));
   ofLine(position,mY,position,mY+mHeight);
   ofPopStyle();
}

int SampleDrawer::GetSampleAtMouse(int x, int y)
{
   if (x >= mX && y >= mY && x <= mX + mWidth && y <= mY + mHeight)
      return (int)ofMap(x,mX,mX+mWidth,mStartSample,mEndSample);
   return -1;
}

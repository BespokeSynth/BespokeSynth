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
   SampleDrawer() : mSample(nullptr), mStartSample(0), mEndSample(0), mX(0), mY(0), mWidth(1), mHeight(1) {}
   void SetSample(Sample* sample) { mSample = sample; }
   void SetPosition(float x, float y) { mX = x; mY = y; }
   void SetDimensions(float w, float h) { mWidth = w; mHeight = h; }
   void SetRange(int startSample, int endSample) { mStartSample = startSample; mEndSample = endSample; }
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

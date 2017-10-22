//
//  Lissajous.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#include "Lissajous.h"
#include "ModularSynth.h"
#include "Profiler.h"

Lissajous::Lissajous()
: IAudioProcessor(gBufferSize)
, mOffset(0)
, mSingleInputMode(true)
, mWidth(500)
, mHeight(500)
{
	for (int i=0; i<NUM_LISSAJOUS_POINTS; ++i)
      mLissajousPoints[i].set(0, 0);
}

Lissajous::~Lissajous()
{
}

void Lissajous::Process(double time)
{
   Profiler profiler("Lissajous");
   
   if (!mEnabled)
      return;
   
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }
   
   mSingleInputMode = (GetBuffer()->NumActiveChannels() == 1);
   int secondChannel = mSingleInputMode ? 0 : 1;
   
   for (int i=0; i<bufferSize; ++i)
      mLissajousPoints[(mOffset+i) % NUM_LISSAJOUS_POINTS].set(GetBuffer()->GetChannel(0)[i],GetBuffer()->GetChannel(secondChannel)[i]);
   
   GetBuffer()->Reset();
   
   mOffset += bufferSize;
   mOffset %= NUM_LISSAJOUS_POINTS;
}

void Lissajous::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofPushStyle();
   ofPushMatrix();
   
   ofSetLineWidth(2);
   
   int w,h;
   GetDimensions(w,h);
   
   ofBeginShape();
   
   const int singleInputModeDelay = 90;
   const float scale = 1.0f;
   
   for (int i=mOffset;i < NUM_LISSAJOUS_POINTS+mOffset-singleInputModeDelay; ++i)
   {
      float x = w/2 + mLissajousPoints[i%NUM_LISSAJOUS_POINTS].x * w * scale;
      float y;
      if (mSingleInputMode)
         y = h/2 + mLissajousPoints[(i+singleInputModeDelay)%NUM_LISSAJOUS_POINTS].x * h * scale;
      else
         y = h/2 + mLissajousPoints[i%NUM_LISSAJOUS_POINTS].y * h * scale;
      //float alpha = (i-mOffset)/float(NUM_LISSAJOUS_POINTS-singleInputModeDelay);
      ofVertex(x,y);
   }
   
   ofEndShape();
   
   ofPopMatrix();
	ofPopStyle();
}

void Lissajous::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}

void Lissajous::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("width", moduleInfo, 500);
   mModuleSaveData.LoadFloat("height", moduleInfo, 500);
   
   SetUpFromSaveData();
}

void Lissajous::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void Lissajous::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mWidth = mModuleSaveData.GetFloat("width");
   mHeight = mModuleSaveData.GetFloat("height");
}

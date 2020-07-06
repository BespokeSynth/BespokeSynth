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
, mAutocorrelationMode(true)
, mWidth(500)
, mHeight(500)
, mOnlyHasOneChannel(true)
, mScaleSlider(nullptr)
, mScale(1)
{
	for (int i=0; i<NUM_LISSAJOUS_POINTS; ++i)
      mLissajousPoints[i].set(0, 0);
}

Lissajous::~Lissajous()
{
}

void Lissajous::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mScaleSlider = new FloatSlider(this, "scale", 0, 0, 100, 15, &mScale, .5f, 4);
}

void Lissajous::Process(double time)
{
   PROFILER(Lissajous);
   
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
   
   mOnlyHasOneChannel = (GetBuffer()->NumActiveChannels() == 1);
   int secondChannel = mOnlyHasOneChannel ? 0 : 1;
   
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
   
   mScaleSlider->Draw();
   
   ofPushStyle();
   ofPushMatrix();
   
   ofSetLineWidth(2);
   
   float w,h;
   GetDimensions(w,h);
   
   ofBeginShape();
   
   const int autocorrelationDelay = 90;
   
   ofSetColor(0,255,0, 30);
   for (int i=mOffset;i < NUM_LISSAJOUS_POINTS+mOffset-autocorrelationDelay; ++i)
   {
      float x = w/2 + mLissajousPoints[i%NUM_LISSAJOUS_POINTS].x * w * mScale;
      float y;
      if (mAutocorrelationMode || mOnlyHasOneChannel)
         y = h/2 + mLissajousPoints[(i+autocorrelationDelay)%NUM_LISSAJOUS_POINTS].x * h * mScale;
      else
         y = h/2 + mLissajousPoints[i%NUM_LISSAJOUS_POINTS].y * h * mScale;
      //float alpha = (i-mOffset)/float(NUM_LISSAJOUS_POINTS-autocorrelationDelay);
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
   mModuleSaveData.LoadBool("autocorrelation", moduleInfo, true);
   
   SetUpFromSaveData();
}

void Lissajous::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
   moduleInfo["autocorrelation"] = mAutocorrelationMode;
}

void Lissajous::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mWidth = mModuleSaveData.GetFloat("width");
   mHeight = mModuleSaveData.GetFloat("height");
   mAutocorrelationMode = mModuleSaveData.GetBool("autocorrelation");
}

//
//  WaveformViewer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/19/12.
//
//

#include "WaveformViewer.h"
#include "ModularSynth.h"
#include "Profiler.h"

WaveformViewer::WaveformViewer()
: IAudioProcessor(gBufferSize)
, mPhaseAlign(true)
, mDoubleBufferFlip(false)
, mHueNote(nullptr)
, mHueAudio(nullptr)
, mHueInstrument(nullptr)
, mHueNoteSource(nullptr)
, mSaturation(nullptr)
, mBrightness(nullptr)
, mWidth(600)
, mHeight(100)
, mDrawWaveform(true)
, mDrawCircle(false)
{
   mBufferVizOffset[0] = 0;
   mBufferVizOffset[1] = 0;
   mVizPhase[0] = 0;
   mVizPhase[1] = 0;
   
	for (int i=0; i<BUFFER_VIZ_SIZE; ++i)
   {
      for (int j=0; j<2; ++j)
         mAudioView[i][j] = 0;
   }
}

void WaveformViewer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   /*mHueNote = new FloatSlider(this,"note",5,0,100,15,&IDrawableModule::sHueNote,0,255);
   mHueAudio = new FloatSlider(this,"audio",5,15,100,15,&IDrawableModule::sHueAudio,0,255);
   mHueInstrument = new FloatSlider(this,"instrument",110,0,100,15,&IDrawableModule::sHueInstrument,0,255);
   mHueNoteSource = new FloatSlider(this,"notesource",110,15,100,15,&IDrawableModule::sHueNoteSource,0,255);
   mSaturation = new FloatSlider(this,"saturation",215,0,100,15,&IDrawableModule::sSaturation,0,255);
   mBrightness = new FloatSlider(this,"brightness",215,15,100,15,&IDrawableModule::sBrightness,0,255);*/
}

WaveformViewer::~WaveformViewer()
{
}

void WaveformViewer::Process(double time)
{
   PROFILER(WaveformViewer);
   
   ComputeSliders(0);

   if (!mEnabled)
      return;
   
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (ch == 0)
            BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         else
            Add(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }
   
   for (int i=0; i<bufferSize; ++i)
      mAudioView[(i+mBufferVizOffset[!mDoubleBufferFlip]) % BUFFER_VIZ_SIZE][!mDoubleBufferFlip] = gWorkBuffer[i];
      
   GetBuffer()->Reset();
   
   float vizPhaseInc = GetPhaseInc(gVizFreq);
   mVizPhase[!mDoubleBufferFlip] += vizPhaseInc * bufferSize;
   while (mVizPhase[!mDoubleBufferFlip] > FTWO_PI) { mVizPhase[!mDoubleBufferFlip] -= FTWO_PI; }
   
   mBufferVizOffset[!mDoubleBufferFlip] = (mBufferVizOffset[!mDoubleBufferFlip] + bufferSize) % BUFFER_VIZ_SIZE;
}

void WaveformViewer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   /*mHueNote->Draw();
   mHueAudio->Draw();
   mHueInstrument->Draw();
   mHueNoteSource->Draw();
   mSaturation->Draw();
   mBrightness->Draw();*/

   ofPushStyle();
   ofPushMatrix();
   
   ofSetColor(245, 58, 135);
   ofSetLineWidth(2);
   
   float w,h;
   GetDimensions(w,h);
   int phaseStart = 0;
   int end = BUFFER_VIZ_SIZE - 1;

   while (gVizFreq > 0 && gVizFreq < 50)
      gVizFreq *= 2;

   if (mPhaseAlign)
   {
      float vizPhaseInc = GetPhaseInc(gVizFreq);
      phaseStart = (FTWO_PI - mVizPhase[mDoubleBufferFlip]) / vizPhaseInc;
      end = BUFFER_VIZ_SIZE-(FTWO_PI/vizPhaseInc);
   }
   
   if (mDrawWaveform)
   {
      ofBeginShape();
      for (int i=phaseStart;i < BUFFER_VIZ_SIZE; i++)
      {
         float x = ofMap(i-phaseStart, 0, end, 0, w, true);
         float samp = mAudioView[(i+mBufferVizOffset[mDoubleBufferFlip])%BUFFER_VIZ_SIZE][mDoubleBufferFlip];
         samp *= 3;
         if (x<w)
            ofVertex(x, h/2-samp*(h/2));
      }
      ofEndShape(false);
   }
   
   if (mDrawCircle)
   {
      ofSetCircleResolution(32);
      ofSetLineWidth(1);
      for (int i=phaseStart;i < BUFFER_VIZ_SIZE; i++)
      {
         float a = float(i-phaseStart) / end;
         if (a < 1)
         {
            float rad = a * MIN(w,h)/2;
            float samp = mAudioView[(i+mBufferVizOffset[mDoubleBufferFlip])%BUFFER_VIZ_SIZE][mDoubleBufferFlip];
            if (samp > 0)
               ofSetColor(245, 58, 135, ofMap(samp*2,0,1,0,255,true));
            else
               ofSetColor(58, 245, 135, ofMap(-samp*2,0,1,0,255,true));
            ofCircle(w/2,h/2,rad);
         }
      }
   }
   
   ofPopMatrix();
	ofPopStyle();

   for (int i=0; i<BUFFER_VIZ_SIZE; ++i)
      mAudioView[i][mDoubleBufferFlip] = mAudioView[i][!mDoubleBufferFlip];
   mBufferVizOffset[mDoubleBufferFlip] = mBufferVizOffset[!mDoubleBufferFlip];
   mVizPhase[mDoubleBufferFlip] = mVizPhase[!mDoubleBufferFlip];
   mDoubleBufferFlip = !mDoubleBufferFlip;
}

void WaveformViewer::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}

void WaveformViewer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 600, 50, 2000, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, 100, 50, 2000, K(isTextField));
   mModuleSaveData.LoadBool("draw_waveform", moduleInfo, true);
   mModuleSaveData.LoadBool("draw_circle", moduleInfo, false);

   SetUpFromSaveData();
}

void WaveformViewer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void WaveformViewer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
   mDrawWaveform = mModuleSaveData.GetBool("draw_waveform");
   mDrawCircle = mModuleSaveData.GetBool("draw_circle");
}


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
//  WaveformViewer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/19/12.
//
//

#include "WaveformViewer.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"

WaveformViewer::WaveformViewer()
: IAudioProcessor(gBufferSize)
{
   mBufferVizOffset[0] = 0;
   mBufferVizOffset[1] = 0;
   mVizPhase[0] = 0;
   mVizPhase[1] = 0;

   for (int i = 0; i < BUFFER_VIZ_SIZE; ++i)
   {
      for (int j = 0; j < 2; ++j)
         mAudioView[i][j] = 0;
   }
}

void WaveformViewer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDisplayFreqEntry = new TextEntry(this, "freq", 3, 3, 10, &mDisplayFreq, 1, 10000);
   mLengthSamplesSlider = new IntSlider(this, "length", mDisplayFreqEntry, kAnchor_Below, 100, 15, &mLengthSamples, 1024, BUFFER_VIZ_SIZE);
   mDrawGainSlider = new FloatSlider(this, "draw gain", mLengthSamplesSlider, kAnchor_Below, 100, 15, &mDrawGain, .1f, 5);
   mDisplayFreqEntry->DrawLabel(true);
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

   SyncBuffers();

   if (mEnabled)
   {
      ComputeSliders(0);

      int lengthSamples = MIN(mLengthSamples, BUFFER_VIZ_SIZE);

      int bufferSize = GetBuffer()->BufferSize();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (ch == 0)
            BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         else
            Add(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      }

      for (int i = 0; i < bufferSize; ++i)
         mAudioView[(i + mBufferVizOffset[!mDoubleBufferFlip]) % lengthSamples][!mDoubleBufferFlip] = gWorkBuffer[i];

      float vizPhaseInc = GetPhaseInc(mDisplayFreq / 2);
      mVizPhase[!mDoubleBufferFlip] += vizPhaseInc * bufferSize;
      while (mVizPhase[!mDoubleBufferFlip] > FTWO_PI)
      {
         mVizPhase[!mDoubleBufferFlip] -= FTWO_PI;
      }

      mBufferVizOffset[!mDoubleBufferFlip] = (mBufferVizOffset[!mDoubleBufferFlip] + bufferSize) % lengthSamples;
   }

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void WaveformViewer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDisplayFreqEntry->Draw();
   mLengthSamplesSlider->Draw();
   mDrawGainSlider->Draw();
   /*mHueNote->Draw();
   mHueAudio->Draw();
   mHueInstrument->Draw();
   mHueNoteSource->Draw();
   mSaturation->Draw();
   mBrightness->Draw();*/

   if (!mEnabled)
      return;

   ofPushStyle();
   ofPushMatrix();

   ofSetColor(245, 58, 135);
   ofSetLineWidth(2);

   float w, h;
   GetDimensions(w, h);
   int lengthSamples = MIN(mLengthSamples, BUFFER_VIZ_SIZE);
   float vizPhaseInc = GetPhaseInc(mDisplayFreq / 2);
   float phaseStart = (FTWO_PI - mVizPhase[mDoubleBufferFlip]) / vizPhaseInc;
   float end = lengthSamples - (FTWO_PI / vizPhaseInc);

   if (mDrawWaveform)
   {
      ofBeginShape();
      for (int i = phaseStart; i < lengthSamples; i++)
      {
         float x = ofMap(i - phaseStart, 0, end, 0, w, true);
         float samp = mAudioView[(i + mBufferVizOffset[mDoubleBufferFlip]) % lengthSamples][mDoubleBufferFlip];
         samp *= mDrawGain;
         if (x < w)
            ofVertex(x, h / 2 - samp * (h / 2));
      }
      ofEndShape(false);
   }

   if (mDrawCircle)
   {
      ofSetCircleResolution(32);
      ofSetLineWidth(1);
      for (int i = phaseStart; i < lengthSamples; i++)
      {
         float a = float(i - phaseStart) / end;
         if (a < 1)
         {
            float rad = a * MIN(w, h) / 2;
            float samp = mAudioView[(i + mBufferVizOffset[mDoubleBufferFlip]) % lengthSamples][mDoubleBufferFlip];
            if (samp > 0)
               ofSetColor(245, 58, 135, ofMap(samp * mDrawGain / 10, 0, 1, 0, 255, true));
            else
               ofSetColor(58, 245, 135, ofMap(-samp * mDrawGain / 10, 0, 1, 0, 255, true));
            ofCircle(w / 2, h / 2, rad);
         }
      }
   }

   ofPopMatrix();
   ofPopStyle();

   for (int i = 0; i < lengthSamples; ++i)
      mAudioView[i][mDoubleBufferFlip] = mAudioView[i][!mDoubleBufferFlip];
   mBufferVizOffset[mDoubleBufferFlip] = mBufferVizOffset[!mDoubleBufferFlip];
   mVizPhase[mDoubleBufferFlip] = mVizPhase[!mDoubleBufferFlip];
   mDoubleBufferFlip = !mDoubleBufferFlip;
}

void WaveformViewer::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      float floatPitch = note.pitch;
      if (note.modulation.pitchBend != nullptr)
         floatPitch += note.modulation.pitchBend->GetValue(0);
      mDisplayFreq = TheScale->PitchToFreq(floatPitch);
   }
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
   mModuleSaveData.LoadInt("height", moduleInfo, 150, 50, 2000, K(isTextField));
   mModuleSaveData.LoadBool("draw_waveform", moduleInfo, true);
   mModuleSaveData.LoadBool("draw_circle", moduleInfo, false);

   SetUpFromSaveData();
}

void WaveformViewer::SaveLayout(ofxJSONElement& moduleInfo)
{
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

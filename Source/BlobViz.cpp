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
//  BlobViz.cpp
//  Bespoke
//

#include "BlobViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"

BlobViz::BlobViz()
: IAudioProcessor(gBufferSize)
{
   mWidth = 320;
   mHeight = 260;
   mHistory.resize(kHistory);
}

BlobViz::~BlobViz()
{
}

void BlobViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSensitivitySlider = new FloatSlider(this, "sensitivity", 3, 3, 92, 14, &mSensitivity, 0.1f, 6.0f);
   mTrailSlider = new FloatSlider(this, "trail", 3, 20, 92, 14, &mTrailDecay, 0.0f, 0.98f);
   mGlowSlider = new FloatSlider(this, "glow", 3, 37, 92, 14, &mGlow, 0.2f, 3.0f);
   mGrainSlider = new FloatSlider(this, "grain", 3, 54, 92, 14, &mGrain, 0.0f, 0.5f);
   mHueShiftSlider = new FloatSlider(this, "hue", 3, 71, 92, 14, &mHueShift, 0.0f, 1.0f);
   mNumBlobsSlider = new IntSlider(this, "blobs", 3, 88, 92, 14, &mNumBlobs, 1, kMaxBlobs);
   mSymmetrySlider = new IntSlider(this, "symmetry", 3, 105, 92, 14, &mSymmetry, 1, 8);
   mPaletteSelector = new DropdownList(this, "palette", 3, 122, &mPaletteIndex, 92);
   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);
}

void BlobViz::PaletteColor(float t, float& rOut, float& gOut, float& bOut) const
{
   VizPaletteColor(mPaletteIndex, t, mHueShift, rOut, gOut, bOut);
}

void BlobViz::Process(double time)
{
   PROFILER(BlobViz);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();

   float sumSq = 0;
   float diffSq = 0;
   float prev = 0;
   float* ch0 = GetBuffer()->GetChannel(0);
   for (int i = 0; i < bufferSize; ++i)
   {
      float s = ch0[i];
      sumSq += s * s;
      float d = s - prev;
      diffSq += d * d;
      prev = s;
   }
   float rms = sqrtf(sumSq / MAX(1, bufferSize));
   float high = sqrtf(diffSq / MAX(1, bufferSize));
   const float smoothing = 0.85f;
   mAmplitude = mAmplitude * smoothing + rms * (1 - smoothing);
   mHighEnergy = mHighEnergy * smoothing + high * (1 - smoothing);

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void BlobViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   float w, h;
   GetDimensions(w, h);
   float cx = w * 0.5f;
   float cy = h * 0.5f;

   //dark backdrop with a faint palette-tinted vignette
   ofPushStyle();
   ofFill();
   ofSetColor(6, 6, 10, 255);
   ofRect(0, 0, w, h);
   {
      float br, bgc, bb;
      PaletteColor(0.5f, br, bgc, bb);
      for (int i = 0; i < 3; ++i)
      {
         ofSetColor(br * 255, bgc * 255, bb * 255, 10);
         ofCircle(cx, cy, MAX(w, h) * (0.4f + i * 0.15f));
      }
   }
   ofPopStyle();

   if (mEnabled)
   {
      float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);
      float t = ofClamp(mHighEnergy / (mAmplitude + 0.02f), 0.0f, 1.0f);
      int numBlobs = mNumBlobs;
      if (numBlobs < 1)
         numBlobs = 1;
      if (numBlobs > kMaxBlobs)
         numBlobs = kMaxBlobs;
      int sym = MAX(1, mSymmetry);

      //build this frame's blobs, each on its own slow drift path so they cross and overlap/merge
      Frame& nf = mHistory[mHistoryPos];
      nf.count = numBlobs;
      for (int i = 0; i < numBlobs; ++i)
      {
         float ph = (float)i / numBlobs * FTWO_PI;
         float sp = 0.0005f + i * 0.00018f;
         Blob& bl = nf.blobs[i];
         bl.x = cx + sinf((float)(gTime * sp) + ph) * (0.35f + amp) * w * 0.22f;
         bl.y = cy + cosf((float)(gTime * (sp + 0.00022f)) + ph * 1.3f) * (0.35f + amp) * h * 0.22f;
         bl.radius = (0.05f + amp * 0.38f) * MIN(w, h) * (0.7f + 0.3f * sinf(ph));
         PaletteColor(ofClamp(t + i * 0.12f, 0.0f, 1.0f), bl.r, bl.g, bl.b);
      }
      mHistoryPos = (mHistoryPos + 1) % kHistory;
      if (mHistoryCount < kHistory)
         ++mHistoryCount;

      //draw trail oldest -> newest; older frames expand + fade. Each blob is layered translucent
      //circles (soft glow); overlapping blobs blend into organic "cells". The whole thing is
      //replicated around the centre 'sym' times for a kaleidoscope.
      ofPushStyle();
      ofFill();
      const int layers = 5;
      for (int age = mHistoryCount - 1; age >= 0; --age)
      {
         int fidx = ((mHistoryPos - 1 - age) % kHistory + kHistory) % kHistory;
         const Frame& fr = mHistory[fidx];
         float alpha = powf(mTrailDecay, (float)age);
         if (alpha < 0.008f)
            continue;
         float expand = 1.0f + age * 0.05f;
         for (int bi = 0; bi < fr.count; ++bi)
         {
            const Blob& b = fr.blobs[bi];
            float dx = b.x - cx;
            float dy = b.y - cy;
            float baseR = b.radius * expand;
            for (int k = 0; k < sym; ++k)
            {
               float ang = (float)k / sym * FTWO_PI;
               float ca = cosf(ang);
               float sa = sinf(ang);
               float bx = cx + dx * ca - dy * sa;
               float by = cy + dx * sa + dy * ca;
               for (int layer = 0; layer < layers; ++layer)
               {
                  float lf = (float)layer / (layers - 1);
                  float lr = baseR * (0.35f + lf * 1.25f);
                  float la = alpha * (1.0f - lf) * 0.45f * mGlow;
                  ofSetColor(b.r * 255, b.g * 255, b.b * 255, ofClamp(la, 0.0f, 1.0f) * 200);
                  ofCircle(bx, by, lr);
               }
            }
         }
      }

      //hot cores on the freshest frame's blobs
      {
         int fidx = ((mHistoryPos - 1) % kHistory + kHistory) % kHistory;
         const Frame& fr = mHistory[fidx];
         for (int bi = 0; bi < fr.count; ++bi)
         {
            const Blob& b = fr.blobs[bi];
            float dx = b.x - cx;
            float dy = b.y - cy;
            for (int k = 0; k < sym; ++k)
            {
               float ang = (float)k / sym * FTWO_PI;
               float ca = cosf(ang);
               float sa = sinf(ang);
               float bx = cx + dx * ca - dy * sa;
               float by = cy + dx * sa + dy * ca;
               ofSetColor(b.r * 255, b.g * 255, b.b * 255, 180);
               ofCircle(bx, by, b.radius * 0.5f);
               ofSetColor(255, 255, 255, ofClamp(0.5f * mGlow, 0.0f, 1.0f) * 200);
               ofCircle(bx, by, b.radius * 0.25f);
            }
         }
      }
      ofPopStyle();

      //film grain - composited on top, outside any feedback
      if (mGrain > 0.001f)
      {
         ofPushStyle();
         ofFill();
         int n = (int)(mGrain * w * h * 0.02f);
         for (int i = 0; i < n; ++i)
         {
            ofSetColor(255, 255, 255, ofRandom(8, 36));
            ofRect(ofRandom(0, w), ofRandom(0, h), 1, 1);
         }
         ofPopStyle();
      }
   }

   //controls drawn on top
   mSensitivitySlider->Draw();
   mTrailSlider->Draw();
   mGlowSlider->Draw();
   mGrainSlider->Draw();
   mHueShiftSlider->Draw();
   mNumBlobsSlider->Draw();
   mSymmetrySlider->Draw();
   mPaletteSelector->Draw();
}

void BlobViz::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void BlobViz::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void BlobViz::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

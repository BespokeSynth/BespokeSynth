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
//  CheckerBox.cpp
//  Bespoke
//

#include "CheckerBox.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"
#include <cmath>

CheckerBox::CheckerBox()
: IAudioProcessor(gBufferSize)
{
   mWidth = 360;
   mHeight = 300;
}

CheckerBox::~CheckerBox()
{
}

void CheckerBox::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mSpeedSlider = new FloatSlider(this, "speed", 3, y, 100, 14, &mSpeed, 0.0f, 30.0f);
   y += 17;
   mGridSlider = new IntSlider(this, "grid", 3, y, 100, 14, &mGridN, 2, 24);
   y += 17;
   mPatternSelector = new DropdownList(this, "pattern", 3, y, &mPattern, 100);
   y += 17;
   mDistortSlider = new FloatSlider(this, "distort", 3, y, 100, 14, &mDistort, 0.0f, 1.0f);
   y += 17;
   mChromaSlider = new FloatSlider(this, "chroma", 3, y, 100, 14, &mChroma, 0.0f, 1.0f);
   y += 17;
   mGrainSlider = new FloatSlider(this, "grain", 3, y, 100, 14, &mGrain, 0.0f, 0.4f);
   y += 17;
   mExposureSlider = new FloatSlider(this, "exposure", 3, y, 100, 14, &mExposure, 0.0f, 3.0f);
   y += 17;
   mSensitivitySlider = new FloatSlider(this, "react", 3, y, 100, 14, &mSensitivity, 0.0f, 4.0f);
   y += 17;
   mColorModeSelector = new DropdownList(this, "color", 3, y, &mColorMode, 100);
   y += 17;
   mHueShiftSlider = new FloatSlider(this, "hue", 3, y, 100, 14, &mHueShift, 0.0f, 1.0f);
   y += 17;
   mPaletteSelector = new DropdownList(this, "palette", 3, y, &mPaletteIndex, 100);

   mPatternSelector->AddLabel("grid", kPattern_Grid);
   mPatternSelector->AddLabel("bend", kPattern_Bend);
   mPatternSelector->AddLabel("spiral", kPattern_Spiral);
   mPatternSelector->AddLabel("tunnel", kPattern_Tunnel);

   mColorModeSelector->AddLabel("black/white", 0);
   mColorModeSelector->AddLabel("palette", 1);

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);
}

void CheckerBox::CellColor(int i, int j, int step, float& rOut, float& gOut, float& bOut) const
{
   bool on = (((i + j + step) & 1) == 0);
   if (mColorMode == 0)
   {
      float v = on ? 1.0f : 0.06f;
      rOut = gOut = bOut = v;
   }
   else
   {
      //two contrasting samples of the palette; the flip swaps them
      VizPaletteColor(mPaletteIndex, on ? 0.15f : 0.65f, mHueShift, rOut, gOut, bOut);
   }
   VizExpose(mExposure, rOut, gOut, bOut); //brightness / exposure
}

void CheckerBox::Process(double time)
{
   PROFILER(CheckerBox);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   float sumSq = 0;
   float* ch0 = GetBuffer()->GetChannel(0);
   for (int i = 0; i < bufferSize; ++i)
      sumSq += ch0[i] * ch0[i];
   float rms = sqrtf(sumSq / MAX(1, bufferSize));
   mAmplitude = mAmplitude * 0.85f + rms * 0.15f;

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

void CheckerBox::DrawPattern(float viewX, float viewY, float viewW, float viewH, int step, float amp,
                             float offX, float offY, int channel, float alpha)
{
   int n = ofClamp(mGridN, 2, 24);
   float cx = viewX + viewW * 0.5f + offX;
   float cy = viewY + viewH * 0.5f + offY;
   float t = (float)(gTime * 0.001);
   float distort = mDistort * (0.7f + amp * 1.0f);

   ofFill();

   if (mPattern == kPattern_Grid || mPattern == kPattern_Bend)
   {
      float bend = (mPattern == kPattern_Bend) ? distort : distort * 0.25f;
      for (int j = 0; j < n; ++j)
      {
         for (int i = 0; i < n; ++i)
         {
            //four corners, warped by a barrel/wave field
            float qx[4], qy[4];
            const int cx4[4] = { 0, 1, 1, 0 };
            const int cy4[4] = { 0, 0, 1, 1 };
            for (int k = 0; k < 4; ++k)
            {
               float gx = (i + cx4[k]) / (float)n; //0..1
               float gy = (j + cy4[k]) / (float)n;
               float nx = gx - 0.5f, ny = gy - 0.5f;
               float r2 = nx * nx + ny * ny;
               float warp = 1.0f + bend * (0.9f * r2 * 4.0f) * (0.6f + 0.4f * sinf(t * 2.0f));
               float wx = nx * warp + bend * 0.06f * sinf(t * 3.0f + gy * 10.0f);
               float wy = ny * warp + bend * 0.06f * sinf(t * 2.5f + gx * 10.0f);
               qx[k] = viewX + viewW * 0.5f + wx * viewW + offX;
               qy[k] = viewY + viewH * 0.5f + wy * viewH + offY;
            }
            float r, g, b;
            CellColor(i, j, step, r, g, b);
            if (channel == 1)
            {
               g = 0;
               b = 0;
            }
            else if (channel == 2)
            {
               r = 0;
               g = 0;
            }
            ofSetColor(r * 255, g * 255, b * 255, alpha);
            ofBeginShape();
            for (int k = 0; k < 4; ++k)
               ofVertex(qx[k], qy[k]);
            ofEndShape(true);
         }
      }
   }
   else
   {
      //polar: rings x sectors. spiral = twist by ring; tunnel = exponential zoom + rotation
      float maxR = MIN(viewW, viewH) * 0.5f;
      int rings = n;
      int sectors = MAX(6, n);
      float twist = distort * 3.0f;
      float rot = (mPattern == kPattern_Tunnel) ? t * (1.0f + amp) * 1.5f : t * 0.2f;
      for (int i = 0; i < rings; ++i)
      {
         float f0 = i / (float)rings;
         float f1 = (i + 1) / (float)rings;
         //tunnel packs rings toward the centre (perspective zoom); spiral spaces them evenly
         if (mPattern == kPattern_Tunnel)
         {
            f0 = powf(f0, 2.2f);
            f1 = powf(f1, 2.2f);
         }
         float r0 = maxR * f0;
         float r1 = maxR * f1;
         float twist0 = twist * f0;
         float twist1 = twist * f1;
         for (int j = 0; j < sectors; ++j)
         {
            float a0 = (j / (float)sectors) * FTWO_PI + rot;
            float a1 = ((j + 1) / (float)sectors) * FTWO_PI + rot;
            float qx[4], qy[4];
            //inner-left, inner-right along ring0; outer-right, outer-left along ring1
            qx[0] = cx + cosf(a0 + twist0) * r0;
            qy[0] = cy + sinf(a0 + twist0) * r0;
            qx[1] = cx + cosf(a1 + twist0) * r0;
            qy[1] = cy + sinf(a1 + twist0) * r0;
            qx[2] = cx + cosf(a1 + twist1) * r1;
            qy[2] = cy + sinf(a1 + twist1) * r1;
            qx[3] = cx + cosf(a0 + twist1) * r1;
            qy[3] = cy + sinf(a0 + twist1) * r1;
            float r, g, b;
            CellColor(i, j, step, r, g, b);
            if (channel == 1)
            {
               g = 0;
               b = 0;
            }
            else if (channel == 2)
            {
               r = 0;
               g = 0;
            }
            ofSetColor(r * 255, g * 255, b * 255, alpha);
            ofBeginShape();
            for (int k = 0; k < 4; ++k)
               ofVertex(qx[k], qy[k]);
            ofEndShape(true);
         }
      }
   }
}

void CheckerBox::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   float w, h;
   GetDimensions(w, h);

   ofPushStyle();
   ofFill();
   ofSetColor(6, 7, 12, 255);
   ofRect(0, 0, w, h);
   ofPopStyle();

   const float panelW = 108;
   const float viewX = panelW + 4;
   const float viewY = 3;
   const float viewW = MAX(20.0f, w - viewX - 4);
   const float viewH = MAX(20.0f, h - viewY - 4);

   float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);

   //the flip step advances with real time * speed (and a nudge from the audio)
   int step = (int)((float)(gTime * 0.001) * mSpeed * 4.0f * (1.0f + amp));

   ofPushStyle();
   if (mChroma > 0.01f)
   {
      //chromatic aberration: red & blue passes offset either side, additive, plus the full pass
      float dx = mChroma * (viewW / MAX(2, mGridN)) * 0.9f + amp * 6.0f;
      DrawPattern(viewX, viewY, viewW, viewH, step, amp, -dx, 0, 1, 150); //red
      DrawPattern(viewX, viewY, viewW, viewH, step, amp, dx, 0, 2, 150); //blue
      DrawPattern(viewX, viewY, viewW, viewH, step, amp, 0, 0, 0, 190); //full, slightly translucent
   }
   else
   {
      DrawPattern(viewX, viewY, viewW, viewH, step, amp, 0, 0, 0, 255);
   }
   ofPopStyle();

   //grain
   if (mGrain > 0.001f)
   {
      ofPushStyle();
      ofFill();
      int gn = (int)(mGrain * viewW * viewH * 0.015f);
      for (int i = 0; i < gn; ++i)
      {
         ofSetColor(255, 255, 255, ofRandom(6, 26));
         ofRect(viewX + ofRandom(0, viewW), viewY + ofRandom(0, viewH), 1, 1);
      }
      ofPopStyle();
   }

   mSpeedSlider->Draw();
   mGridSlider->Draw();
   mPatternSelector->Draw();
   mDistortSlider->Draw();
   mChromaSlider->Draw();
   mGrainSlider->Draw();
   mExposureSlider->Draw();
   mSensitivitySlider->Draw();
   mColorModeSelector->Draw();
   mHueShiftSlider->Draw();
   mPaletteSelector->Draw();
}

void CheckerBox::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void CheckerBox::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void CheckerBox::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

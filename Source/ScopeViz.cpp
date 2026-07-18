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
//  ScopeViz.cpp
//  Bespoke
//

#include "ScopeViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "juce_opengl/juce_opengl.h"
using namespace juce::gl;
#include "nanovg/nanovg.h"
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"
#include <cmath>

ScopeViz::ScopeViz()
: IAudioProcessor(gBufferSize)
{
   mWidth = 380;
   mHeight = 300;
   mScopeL.assign(kScopeSize, 0.0f);
   mScopeQ.assign(kScopeSize, 0.0f);

   //ideal Hilbert impulse response, Hamming-windowed -> a 90 deg phase shifter.
   //h[m] = 0 for even m, 2/(pi*m) for odd m (m relative to the centre tap)
   mHil.assign(kHilbertLen, 0.0f);
   for (int k = 0; k < kHilbertLen; ++k)
   {
      int m = k - kHilbertMid;
      float v = 0.0f;
      if (m != 0 && (m % 2 != 0))
         v = 2.0f / (FPI * m);
      float win = 0.54f - 0.46f * cosf(FTWO_PI * k / (kHilbertLen - 1)); //Hamming
      mHil[k] = v * win;
   }
}

ScopeViz::~ScopeViz()
{
   VizGL::DestroyFbo(mOutputFbo);
}

void ScopeViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mGainSlider = new FloatSlider(this, "gain", 3, y, 104, 14, &mGain, 0.1f, 8.0f);
   y += 17;
   mGlowSlider = new FloatSlider(this, "glow", 3, y, 104, 14, &mGlow, 0.0f, 1.0f);
   y += 17;
   mScaleSlider = new FloatSlider(this, "size", 3, y, 104, 14, &mScaleAmt, 0.2f, 1.4f);
   y += 17;
   mExposureSlider = new FloatSlider(this, "exposure", 3, y, 104, 14, &mExposure, 0.0f, 3.0f);
   y += 17;
   mDetailSlider = new IntSlider(this, "trace", 3, y, 104, 14, &mDetail, 64, 4000);
   y += 17;
   mGrainSlider = new FloatSlider(this, "grain", 3, y, 104, 14, &mGrain, 0.0f, 0.3f);
   y += 17;
   mHueShiftSlider = new FloatSlider(this, "hue", 3, y, 104, 14, &mHueShift, 0.0f, 1.0f);
   y += 17;
   mGridCheckbox = new Checkbox(this, "grid", 3, y, &mShowGrid);
   y += 17;
   mPaletteSelector = new DropdownList(this, "palette", 3, y, &mPaletteIndex, 104);
   y += 17;

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);
}

void ScopeViz::GlowLine(float x0, float y0, float x1, float y1, float r, float g, float b, float bright, float glow)
{
   if (glow > 0.01f)
   {
      ofSetLineWidth(1.0f + glow * 4.0f);
      ofSetColor(r * 255, g * 255, b * 255, ofClamp(40.0f * glow * bright, 0.0f, 110.0f));
      ofLine(x0, y0, x1, y1);
   }
   ofSetLineWidth(1.4f);
   ofSetColor(r * 255, g * 255, b * 255, ofClamp(220.0f * bright, 25.0f, 255.0f));
   ofLine(x0, y0, x1, y1);
}

void ScopeViz::Process(double time)
{
   PROFILER(ScopeViz);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   float* l = GetBuffer()->GetChannel(0);

   float sumSq = 0;
   for (int i = 0; i < bufferSize; ++i)
   {
      int p = mWritePos;
      mScopeL[p] = l[i];

      //Hilbert (90 deg) of the mono input at this sample, using the stored history
      float q = 0.0f;
      for (int k = 0; k < kHilbertLen; ++k)
         q += mHil[k] * mScopeL[(p - k + kScopeSize) % kScopeSize];
      mScopeQ[p] = q;

      sumSq += l[i] * l[i];
      mWritePos = (p + 1) % kScopeSize;
   }
   float rms = sqrtf(sumSq / MAX(1, bufferSize));
   mAmplitude = mAmplitude * 0.8f + rms * 0.2f;

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

unsigned int ScopeViz::GetOutputTexture()
{
   return mOutputFbo.fb ? mOutputFbo.fb->texture : 0;
}

void ScopeViz::CookIfNeeded(int frameId)
{
   if (mLastCookFrame == frameId && frameId != 0)
      return;
   mLastCookFrame = frameId;

   NVGcontext* recVG = gNanoVGRenderContexts[(int)NanoVGRenderContext::Screenshot];
   if (!recVG)
      return;
   int w = MAX(1080, ((int)mWidth & ~1));
   int h = MAX(1080, ((int)mHeight & ~1));

   NVGcontext* mainVG = gNanoVG;
   gNanoVG = recVG;

   VizGL::EnsureFbo(mOutputFbo, w, h);

   GLint prevFBO = 0;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
   GLint prevVp[4];
   glGetIntegerv(GL_VIEWPORT, prevVp);

   VizGL::BindFbo(mOutputFbo);
   glViewport(0, 0, w, h);
   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(gNanoVG, w, h, 1);

   ofPushStyle();
   ofFill();
   ofSetColor(4, 7, 6, 255);
   ofRect(0, 0, w, h);
   ofPopStyle();

   const float panelW = 112;
   const float viewX = panelW + 4;
   const float viewY = 3;
   const float viewW = MAX(20.0f, w - viewX - 4);
   const float viewH = MAX(20.0f, h - viewY - 4);
   const float cx = viewX + viewW * 0.5f;
   const float cy = viewY + viewH * 0.5f;
   const float rad = MIN(viewW, viewH) * 0.5f * mScaleAmt;

   //trace colour (single hue, like a real phosphor tube; palette + hue pick the colour)
   float tr, tg, tb;
   VizPaletteColor(mPaletteIndex, 0.55f, mHueShift, tr, tg, tb);
   VizExpose(mExposure, tr, tg, tb);

   //faint graticule
   if (mShowGrid)
   {
      ofPushStyle();
      ofSetColor(tr * 255, tg * 255, tb * 255, 26);
      ofSetLineWidth(1.0f);
      for (int i = -4; i <= 4; ++i)
      {
         float gx = cx + i * (MIN(viewW, viewH) * 0.5f / 4.0f);
         float gy = cy + i * (MIN(viewW, viewH) * 0.5f / 4.0f);
         ofLine(gx, viewY, gx, viewY + viewH);
         ofLine(viewX, gy, viewX + viewW, gy);
      }
      ofPopStyle();
   }

   //silence -> draw nothing (a real scope shows no trace with no signal)
   const bool hasSignal = mAmplitude > 0.0009f;
   if (hasSignal)
   {
      int n = ofClamp(mDetail, 8, kScopeSize - 1);
      int start = (mWritePos - n + kScopeSize) % kScopeSize;
      float g = mGain;

      auto sampleXY = [&](int idx, float& sx, float& sy)
      {
         //X = signal (delayed to align with the Hilbert group delay), Y = 90 deg-shifted signal
         float vx = mScopeL[(idx - kHilbertMid + kScopeSize) % kScopeSize];
         float vy = mScopeQ[idx];
         sx = cx + ofClamp(vx * g, -1.2f, 1.2f) * rad;
         sy = cy - ofClamp(vy * g, -1.2f, 1.2f) * rad;
      };

      ofPushStyle();
      float px, py;
      sampleXY(start, px, py);
      for (int i = 1; i < n; ++i)
      {
         int idx = (start + i) % kScopeSize;
         float sx, sy;
         sampleXY(idx, sx, sy);
         //phosphor: the slower the beam moves (short step), the brighter the trace
         float d = sqrtf((sx - px) * (sx - px) + (sy - py) * (sy - py));
         float bright = ofClamp(1.0f - d / (rad * 0.5f + 1.0f), 0.10f, 1.0f);
         GlowLine(px, py, sx, sy, tr, tg, tb, bright, mGlow);
         px = sx;
         py = sy;
      }
      ofPopStyle();
   }

   //grain
   if (mGrain > 0.001f)
   {
      ofPushStyle();
      ofFill();
      int gn = (int)(mGrain * viewW * viewH * 0.015f);
      for (int i = 0; i < gn; ++i)
      {
         ofSetColor(255, 255, 255, ofRandom(6, 20));
         ofRect(viewX + ofRandom(0, viewW), viewY + ofRandom(0, viewH), 1, 1);
      }
      ofPopStyle();
   }

   nvgEndFrame(gNanoVG);
   VizGL::UnbindFbo();
   glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
   glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
   gNanoVG = mainVG;
}

void ScopeViz::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   CookIfNeeded(0);

   VizGL::DrawTexture(mOutputFbo.fb ? mOutputFbo.fb->texture : 0, 0, 0, mWidth, mHeight);

   mGainSlider->Draw();
   mGlowSlider->Draw();
   mScaleSlider->Draw();
   mExposureSlider->Draw();
   mDetailSlider->Draw();
   mGrainSlider->Draw();
   mHueShiftSlider->Draw();
   mGridCheckbox->Draw();
   mPaletteSelector->Draw();
}

void ScopeViz::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void ScopeViz::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ScopeViz::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

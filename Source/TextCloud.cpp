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
//  TextCloud.cpp
//  Bespoke
//

#include "TextCloud.h"
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
#include <cstring>

namespace
{
   const int kMaxVoxels = 2200; //decimate the rasterised text down to this many points
   const int kMaxBasePoints = 5200; //voxels * depth-layers ceiling

   //common cross-platform typefaces; "default" uses the app's sans font
   const char* kFontNames[] = { "default", "Arial", "Helvetica", "Times New Roman", "Georgia",
                                "Courier New", "Verdana", "Impact", "Trebuchet MS", "Menlo" };
   const int kNumFonts = (int)(sizeof(kFontNames) / sizeof(kFontNames[0]));
}

TextCloud::TextCloud()
: IAudioProcessor(gBufferSize)
{
   mWidth = 380;
   mHeight = 300;
   strcpy(mText, "HELLO");
}

TextCloud::~TextCloud()
{
   VizGL::DestroyFbo(mOutputFbo);
}

void TextCloud::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mTextEntry = new TextEntry(this, "text", 3, y, 16, mText);
   mTextEntry->SetRequireEnter(false);
   y += 18;
   mExtrudeSlider = new FloatSlider(this, "depth", 3, y, 108, 14, &mExtrude, 0.0f, 1.0f);
   y += 17;
   mSpinSlider = new FloatSlider(this, "spin", 3, y, 108, 14, &mSpinSpeed, 0.0f, 3.0f);
   y += 17;
   mSensitivitySlider = new FloatSlider(this, "react", 3, y, 108, 14, &mSensitivity, 0.0f, 4.0f);
   y += 17;
   mDistortSlider = new FloatSlider(this, "distort", 3, y, 108, 14, &mDistort, 0.0f, 1.0f);
   y += 17;
   mBlurSlider = new FloatSlider(this, "blur", 3, y, 108, 14, &mBlur, 0.0f, 1.0f);
   y += 17;
   mBloomSlider = new FloatSlider(this, "bloom", 3, y, 108, 14, &mBloom, 0.0f, 1.0f);
   y += 17;
   mGrainSlider = new FloatSlider(this, "grain", 3, y, 108, 14, &mGrain, 0.0f, 0.4f);
   y += 17;
   mExposureSlider = new FloatSlider(this, "exposure", 3, y, 108, 14, &mExposure, 0.0f, 3.0f);
   y += 17;
   mHueShiftSlider = new FloatSlider(this, "hue", 3, y, 108, 14, &mHueShift, 0.0f, 1.0f);
   y += 17;
   mColorModeSelector = new DropdownList(this, "color", 3, y, &mColorMode, 108);
   y += 17;
   mPaletteSelector = new DropdownList(this, "palette", 3, y, &mPaletteIndex, 108);
   y += 17;
   mFontSelector = new DropdownList(this, "font", 3, y, &mFontIndex, 108);

   for (int i = 0; i < kNumFonts; ++i)
      mFontSelector->AddLabel(kFontNames[i], i);

   mColorModeSelector->AddLabel("palette", 0);
   mColorModeSelector->AddLabel("mono", 1);

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);

   y += 17;
}

void TextCloud::PaletteColor(float t, float& rOut, float& gOut, float& bOut) const
{
   VizPaletteColor(mPaletteIndex, t, mHueShift, rOut, gOut, bOut);
}

void TextCloud::RebuildVoxels()
{
   mVoxels.clear();
   std::string text = mText;
   mLastBuilt = text;
   if (text.empty())
      return;

   mLastFontIndex = mFontIndex;

   //rasterise the text into a small bitmap with the chosen JUCE font, then voxelise the lit pixels
   const int gh = 22;
   int fi = ofClamp(mFontIndex, 0, kNumFonts - 1);
   juce::Font font = (fi == 0) ? juce::Font((float)gh, juce::Font::bold)
                               : juce::Font(juce::String(kFontNames[fi]), (float)gh, juce::Font::bold);
   int gw = (int)ceilf(font.getStringWidth(juce::String(text)));
   gw = (int)ofClamp((float)gw, 1.0f, 420.0f);

   juce::Image img(juce::Image::ARGB, gw, gh, true);
   {
      juce::Graphics g(img);
      g.setColour(juce::Colours::white);
      g.setFont(font);
      g.drawText(juce::String(text), 0, 0, gw, gh, juce::Justification::centredLeft, false);
   }

   mAspectCols = gw;
   mAspectRows = gh;
   float maxDim = (float)MAX(gw, gh);

   //collect lit pixels
   std::vector<Voxel> all;
   all.reserve(gw * gh / 3);
   {
      juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
      for (int r = 0; r < gh; ++r)
      {
         for (int c = 0; c < gw; ++c)
         {
            float a = bmp.getPixelColour(c, r).getFloatAlpha();
            if (a > 0.15f)
            {
               Voxel vx;
               vx.u = (c - gw * 0.5f) / maxDim;
               vx.v = (r - gh * 0.5f) / maxDim;
               vx.lum = ofClamp(a, 0.0f, 1.0f);
               all.push_back(vx);
            }
         }
      }
   }

   //decimate to the voxel cap (keep every Nth) so long strings can't blow up the point count
   if ((int)all.size() > kMaxVoxels)
   {
      int step = (int)ceilf((float)all.size() / kMaxVoxels);
      for (size_t i = 0; i < all.size(); i += step)
         mVoxels.push_back(all[i]);
   }
   else
   {
      mVoxels = all;
   }
}

void TextCloud::Process(double time)
{
   PROFILER(TextCloud);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   float sumSq = 0, diffSq = 0, prev = 0;
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

unsigned int TextCloud::GetOutputTexture()
{
   return mOutputFbo.fb ? mOutputFbo.fb->texture : 0;
}

void TextCloud::CookIfNeeded(int frameId)
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
   ofSetColor(6, 7, 12, 255);
   ofRect(0, 0, w, h);
   ofPopStyle();

   const float panelW = 116;
   const float viewX = panelW + 4;
   const float viewY = 3;
   const float viewW = MAX(20.0f, w - viewX - 4);
   const float viewH = MAX(20.0f, h - viewY - 4);
   const float cx = viewX + viewW * 0.5f;
   const float cy = viewY + viewH * 0.5f;
   const float scale = MIN(viewW, viewH) * 0.85f;

   float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);
   float high = ofClamp(mHighEnergy * mSensitivity * 10.0f, 0.0f, 1.0f);

   if (!mVoxels.empty())
   {
      //depth layers (extrude); keep voxels*layers under the ceiling
      int layers = MAX(1, (int)roundf(mExtrude * 7.0f));
      if ((int)mVoxels.size() * layers > kMaxBasePoints)
         layers = MAX(1, kMaxBasePoints / (int)mVoxels.size());
      float depth = mExtrude * (0.5f + amp * 0.7f);

      const float crx = cosf(mRotX), srx = sinf(mRotX);
      const float cry = cosf(mRotY), sry = sinf(mRotY);
      const float crz = cosf(mRotZ), srz = sinf(mRotZ);
      const float focal = 2.4f;
      float t = (float)(gTime * 0.001);
      float distort = mDistort * (0.6f + amp * 1.2f);

      ofPushStyle();
      ofFill();
      for (const Voxel& vx : mVoxels)
      {
         for (int L = 0; L < layers; ++L)
         {
            float lf = (layers > 1) ? ((float)L / (layers - 1) - 0.5f) : 0.0f; //-0.5..0.5
            float ux = vx.u;
            float uy = vx.v;
            float uz = lf * depth;

            //distortion: sinusoidal wobble driven by position + time + audio
            if (distort > 0.001f)
            {
               ux += distort * 0.12f * sinf(t * 2.0f + vx.v * 9.0f);
               uy += distort * 0.12f * sinf(t * 2.3f + vx.u * 9.0f);
               uz += distort * 0.12f * sinf(t * 1.7f + (vx.u + vx.v) * 7.0f);
            }

            //rotate X, Y, Z
            float y1 = uy * crx - uz * srx, z1 = uy * srx + uz * crx, x1 = ux;
            float x2 = x1 * cry + z1 * sry, z2 = -x1 * sry + z1 * cry, y2 = y1;
            float x3 = x2 * crz - y2 * srz, y3 = x2 * srz + y2 * crz, z3 = z2;

            float persp = focal / (focal - z3);
            float sx = cx + x3 * scale * persp;
            float sy = cy + y3 * scale * persp;

            float shade = ofClamp(0.55f + z3 * 0.6f, 0.25f, 1.0f);
            float colT = 0.5f + z3 * 0.5f;
            float rr, gg, bb;
            if (mColorMode == 1)
            {
               rr = gg = bb = vx.lum;
            }
            else
            {
               PaletteColor(colT, rr, gg, bb);
            }
            VizExpose(mExposure, rr, gg, bb); //brightness / exposure

            float ps = MAX(1.0f, scale / MAX(mAspectCols, mAspectRows) * 1.1f * persp);

            //bloom: soft additive halo on the brighter/near points
            if (mBloom > 0.01f && z3 > -0.1f)
            {
               float bloomA = mBloom * 70.0f * shade;
               ofSetColor(rr * 255, gg * 255, bb * 255, ofClamp(bloomA, 0.0f, 120.0f));
               float bs = ps * (2.0f + mBloom * 2.5f);
               ofRect(sx - bs * 0.5f, sy - bs * 0.5f, bs, bs);
            }

            //blur: one translucent enlarged copy (cheap fake gaussian)
            if (mBlur > 0.01f)
            {
               float bs = ps * (1.6f + mBlur * 1.8f);
               ofSetColor(rr * 255 * shade, gg * 255 * shade, bb * 255 * shade, ofClamp(mBlur * 110.0f, 0.0f, 120.0f));
               ofRect(sx - bs * 0.5f, sy - bs * 0.5f, bs, bs);
            }

            //core point
            ofSetColor(rr * 255 * shade, gg * 255 * shade, bb * 255 * shade, 240);
            ofRect(sx - ps * 0.5f, sy - ps * 0.5f, ps, ps);
         }
      }
      ofPopStyle();

      //grain overlay
      if (mGrain > 0.001f)
      {
         ofPushStyle();
         ofFill();
         int n = (int)(mGrain * viewW * viewH * 0.015f);
         for (int i = 0; i < n; ++i)
         {
            ofSetColor(255, 255, 255, ofRandom(6, 26));
            ofRect(viewX + ofRandom(0, viewW), viewY + ofRandom(0, viewH), 1, 1);
         }
         ofPopStyle();
      }
   }
   else
   {
      ofPushStyle();
      ofSetColor(150, 155, 170, 160);
      DrawTextNormal("type text on the left", viewX + 8, viewY + viewH * 0.5f, 13);
      ofPopStyle();
   }

   //auto-tumble (unless dragging)
   if (mEnabled && !mDragging)
   {
      mRotY += (0.004f + amp * 0.02f) * mSpinSpeed;
      mRotX += (0.0015f + high * 0.01f) * mSpinSpeed;
   }

   nvgEndFrame(gNanoVG);
   VizGL::UnbindFbo();
   glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
   glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
   gNanoVG = mainVG;
}

void TextCloud::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   //rebuild when the typed text changes (live, without needing enter)
   if (mText != mLastBuilt)
      RebuildVoxels();

   CookIfNeeded(0);

   VizGL::DrawTexture(mOutputFbo.fb ? mOutputFbo.fb->texture : 0, 0, 0, mWidth, mHeight);

   mTextEntry->Draw();
   mExtrudeSlider->Draw();
   mSpinSlider->Draw();
   mSensitivitySlider->Draw();
   mDistortSlider->Draw();
   mBlurSlider->Draw();
   mBloomSlider->Draw();
   mGrainSlider->Draw();
   mExposureSlider->Draw();
   mHueShiftSlider->Draw();
   mColorModeSelector->Draw();
   mPaletteSelector->Draw();
   mFontSelector->Draw();
}

void TextCloud::TextEntryComplete(TextEntry* entry)
{
   RebuildVoxels();
}

void TextCloud::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mFontSelector)
      RebuildVoxels();
}

void TextCloud::OnClicked(float x, float y, bool right)
{
   if (!right && x > 120)
   {
      mDragging = true;
      mLastDragX = TheSynth->GetMouseX(GetOwningContainer());
      mLastDragY = TheSynth->GetMouseY(GetOwningContainer());
      return;
   }
   IDrawableModule::OnClicked(x, y, right);
}

void TextCloud::Poll()
{
   if (mDragging)
   {
      if (TheSynth->IsMouseButtonHeld(1))
      {
         float mx = TheSynth->GetMouseX(GetOwningContainer());
         float my = TheSynth->GetMouseY(GetOwningContainer());
         mRotY += (mx - mLastDragX) * 0.01f;
         mRotX += (my - mLastDragY) * 0.01f;
         mLastDragX = mx;
         mLastDragY = my;
      }
      else
      {
         mDragging = false;
      }
   }
}

void TextCloud::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   if (moduleInfo.isMember("text"))
   {
      std::string t = moduleInfo["text"].asString();
      strncpy(mText, t.c_str(), sizeof(mText) - 1);
      mText[sizeof(mText) - 1] = 0;
   }
   SetUpFromSaveData();
}

void TextCloud::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["text"] = std::string(mText);
}

void TextCloud::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   RebuildVoxels();
}

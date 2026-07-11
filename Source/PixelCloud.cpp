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
//  PixelCloud.cpp
//  Bespoke
//

#include "PixelCloud.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"
#include <cmath>

namespace
{
   const int kMaxCells = 13000; //hard ceiling so the frame rate never tanks

   //cheap deterministic hash -> 0..1 (for stable per-row/per-cell glitch without allocations)
   inline float Hash(int x, int y)
   {
      int h = x * 374761393 + y * 668265263;
      h = (h ^ (h >> 13)) * 1274126177;
      return ((h ^ (h >> 16)) & 0x7fffffff) / (float)0x7fffffff;
   }
}

PixelCloud::PixelCloud()
: IAudioProcessor(gBufferSize)
{
   mWidth = 360;
   mHeight = 330;
}

PixelCloud::~PixelCloud()
{
}

void PixelCloud::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mModeSelector = new DropdownList(this, "mode", 3, y, &mMode, 100);
   y += 17;
   mDensitySlider = new IntSlider(this, "density", 3, y, 100, 14, &mDensity, 24, 110);
   y += 17;
   mGlitchSlider = new FloatSlider(this, "glitch", 3, y, 100, 14, &mGlitch, 0.0f, 1.0f);
   y += 17;
   mDepthSlider = new FloatSlider(this, "depth", 3, y, 100, 14, &mDepth, 0.0f, 2.0f);
   y += 17;
   mSpinSlider = new FloatSlider(this, "spin", 3, y, 100, 14, &mSpinSpeed, 0.0f, 3.0f);
   y += 17;
   mSensitivitySlider = new FloatSlider(this, "react", 3, y, 100, 14, &mSensitivity, 0.0f, 4.0f);
   y += 17;
   mGrainSlider = new FloatSlider(this, "grain", 3, y, 100, 14, &mGrain, 0.0f, 0.4f);
   y += 17;
   mHueShiftSlider = new FloatSlider(this, "hue", 3, y, 100, 14, &mHueShift, 0.0f, 1.0f);
   y += 17;
   mColorModeSelector = new DropdownList(this, "color", 3, y, &mColorMode, 100);
   y += 17;
   mPaletteSelector = new DropdownList(this, "palette", 3, y, &mPaletteIndex, 100);

   mModeSelector->AddLabel("ascii", kMode_Ascii);
   mModeSelector->AddLabel("cloud", kMode_Cloud);

   mColorModeSelector->AddLabel("image", 0);
   mColorModeSelector->AddLabel("palette", 1);
   mColorModeSelector->AddLabel("mono", 2);

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);
}

void PixelCloud::PaletteColor(float t, float& rOut, float& gOut, float& bOut) const
{
   VizPaletteColor(mPaletteIndex, t, mHueShift, rOut, gOut, bOut);
}

void PixelCloud::CellColor(int idx, float lum, float& rOut, float& gOut, float& bOut) const
{
   if (mColorMode == 0 && idx >= 0 && idx < (int)mR.size()) //image colours
   {
      rOut = mR[idx];
      gOut = mG[idx];
      bOut = mB[idx];
   }
   else if (mColorMode == 1) //cosine palette keyed by brightness
   {
      PaletteColor(lum, rOut, gOut, bOut);
   }
   else //mono
   {
      rOut = gOut = bOut = lum;
   }
}

bool PixelCloud::LoadImageFile(const std::string& path)
{
   juce::File file(path);
   if (!file.existsAsFile())
      return false;
   juce::Image img = juce::ImageFileFormat::loadFrom(file);
   if (!img.isValid())
      return false;

   //cap the stored resolution (long side <= 1024) so big drops stay fast + light on memory
   const int kMaxDim = 1024;
   int srcW = img.getWidth();
   int srcH = img.getHeight();
   int step = 1;
   while (MAX(srcW, srcH) / step > kMaxDim)
      ++step;
   mImgW = MAX(1, srcW / step);
   mImgH = MAX(1, srcH / step);
   mImgRGB.assign((size_t)mImgW * mImgH * 3, 0);
   {
      juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
      for (int py = 0; py < mImgH; ++py)
      {
         int sy = MIN(srcH - 1, py * step);
         for (int px = 0; px < mImgW; ++px)
         {
            int sx = MIN(srcW - 1, px * step);
            juce::Colour c = bmp.getPixelColour(sx, sy);
            size_t o = ((size_t)py * mImgW + px) * 3;
            mImgRGB[o + 0] = c.getRed();
            mImgRGB[o + 1] = c.getGreen();
            mImgRGB[o + 2] = c.getBlue();
         }
      }
   }
   mImagePath = path;
   RebuildGrid();
   return true;
}

void PixelCloud::RebuildGrid()
{
   mCols = 0;
   mRows = 0;
   mLum.clear();
   mR.clear();
   mG.clear();
   mB.clear();
   if (mImgW <= 0 || mImgH <= 0)
      return;

   int cols = ofClamp(mDensity, 8, 200);
   int rows = MAX(1, (int)roundf(cols * (float)mImgH / (float)mImgW));
   //enforce the cell ceiling by scaling both dimensions down together
   if ((long)cols * rows > kMaxCells)
   {
      float scale = sqrtf((float)kMaxCells / ((float)cols * rows));
      cols = MAX(8, (int)(cols * scale));
      rows = MAX(1, (int)(rows * scale));
   }

   mCols = cols;
   mRows = rows;
   int n = cols * rows;
   mLum.resize(n);
   mR.resize(n);
   mG.resize(n);
   mB.resize(n);

   for (int r = 0; r < rows; ++r)
   {
      int py = ofClamp((int)((r + 0.5f) / rows * mImgH), 0, mImgH - 1);
      for (int c = 0; c < cols; ++c)
      {
         int px = ofClamp((int)((c + 0.5f) / cols * mImgW), 0, mImgW - 1);
         size_t o = ((size_t)py * mImgW + px) * 3;
         float rr = mImgRGB[o + 0] / 255.0f;
         float gg = mImgRGB[o + 1] / 255.0f;
         float bb = mImgRGB[o + 2] / 255.0f;
         int idx = r * cols + c;
         mR[idx] = rr;
         mG[idx] = gg;
         mB[idx] = bb;
         mLum[idx] = ofClamp(0.299f * rr + 0.587f * gg + 0.114f * bb, 0.0f, 1.0f);
      }
   }
}

void PixelCloud::Process(double time)
{
   PROFILER(PixelCloud);

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

void PixelCloud::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   float w, h;
   GetDimensions(w, h);

   //backdrop
   ofPushStyle();
   ofFill();
   ofSetColor(6, 7, 12, 255);
   ofRect(0, 0, w, h);
   ofPopStyle();

   const float panelW = 108; //left control strip
   const float viewX = panelW + 4;
   const float viewY = 3;
   const float viewW = MAX(20.0f, w - viewX - 4);
   const float viewH = MAX(20.0f, h - viewY - 4);

   float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);
   float high = ofClamp(mHighEnergy * mSensitivity * 10.0f, 0.0f, 1.0f);

   if (mCols > 0 && mRows > 0)
   {
      if (mMode == kMode_Ascii)
      {
         //--- glitchy ASCII / halftone block field -------------------------------------------------
         //fit the grid to the view while preserving aspect
         float cellW = viewW / mCols;
         float cellH = viewH / mRows;
         float cell = MIN(cellW, cellH);
         float gridW = cell * mCols;
         float gridH = cell * mRows;
         float ox = viewX + (viewW - gridW) * 0.5f;
         float oy = viewY + (viewH - gridH) * 0.5f;

         float glitch = mGlitch;
         float t = (float)(gTime * 0.001);
         int frameBlock = (int)(gTime * 0.02); //changes a few times a second for chunky tearing
         float splitAmt = glitch * mDepth * cell * 1.5f + amp * cell * 2.0f;

         ofPushStyle();
         for (int r = 0; r < mRows; ++r)
         {
            //per-row horizontal displacement (data-mosh style), stronger with glitch + audio
            float rowNoise = Hash(r, frameBlock);
            float tear = 0.0f;
            if (glitch > 0.001f)
            {
               tear = (rowNoise - 0.5f) * glitch * cell * 6.0f;
               if (rowNoise > 0.93f - glitch * 0.25f) //occasional big slip
                  tear += (Hash(r, frameBlock + 7) - 0.5f) * gridW * 0.4f * glitch;
               tear += sinf(t * 3.0f + r * 0.4f) * glitch * amp * cell * 4.0f;
            }
            float cy = oy + r * cell + cell * 0.5f;

            for (int c = 0; c < mCols; ++c)
            {
               int idx = r * mCols + c;
               float lum = mLum[idx];
               if (lum < 0.12f)
                  continue; //dark -> empty, gives the sparse ASCII look

               //random dropouts scale with glitch
               if (glitch > 0.4f && Hash(c * 3 + 1, r * 5 + frameBlock) < (glitch - 0.4f) * 0.35f)
                  continue;

               float cx = ox + c * cell + cell * 0.5f + tear;
               float rr, gg, bb;
               CellColor(idx, lum, rr, gg, bb);

               //RGB channel split (chromatic aberration) when glitching / loud
               if (splitAmt > 0.5f && mColorMode != 2)
               {
                  ofFill();
                  ofSetColor(rr * 255, 0, 0, 150);
                  ofRect(cx - splitAmt * 0.5f - cell * 0.35f, cy - cell * 0.35f, cell * 0.7f, cell * 0.7f);
                  ofSetColor(0, 0, bb * 255, 150);
                  ofRect(cx + splitAmt * 0.5f - cell * 0.35f, cy - cell * 0.35f, cell * 0.7f, cell * 0.7f);
               }

               //lum-bucketed "glyph": dot / plus / hollow / filled - drawn with cheap primitives
               ofSetColor(rr * 255, gg * 255, bb * 255, 255);
               float s = cell * (0.35f + 0.6f * lum);
               float hs = s * 0.5f;
               if (lum < 0.32f)
               {
                  ofFill();
                  ofRect(cx - 0.5f, cy - 0.5f, MAX(1.0f, cell * 0.2f), MAX(1.0f, cell * 0.2f));
               }
               else if (lum < 0.55f)
               {
                  ofSetLineWidth(1.0f);
                  ofLine(cx - hs, cy, cx + hs, cy);
                  ofLine(cx, cy - hs, cx, cy + hs);
               }
               else if (lum < 0.76f)
               {
                  ofNoFill();
                  ofSetLineWidth(1.0f);
                  ofRect(cx - hs, cy - hs, s, s);
               }
               else
               {
                  ofFill();
                  ofRect(cx - hs, cy - hs, s, s);
               }
            }
         }
         ofPopStyle();
      }
      else
      {
         //--- 3D point cloud ------------------------------------------------------------------------
         float cx = viewX + viewW * 0.5f;
         float cy = viewY + viewH * 0.5f;
         float scale = MIN(viewW, viewH) * 0.9f;

         const float crx = cosf(mRotX), srx = sinf(mRotX);
         const float cry = cosf(mRotY), sry = sinf(mRotY);
         const float crz = cosf(mRotZ), srz = sinf(mRotZ);
         const float focal = 2.2f;
         float depth = mDepth * (0.6f + amp * 0.8f);

         float aspect = (float)mRows / MAX(1, mCols);
         float ptBase = MAX(1.0f, scale / MAX(mCols, mRows) * 0.9f);

         ofPushStyle();
         ofFill();
         for (int r = 0; r < mRows; ++r)
         {
            for (int c = 0; c < mCols; ++c)
            {
               int idx = r * mCols + c;
               float lum = mLum[idx];
               if (lum < 0.10f)
                  continue;

               float ux = (float)c / (mCols - 1) - 0.5f;
               float uy = ((float)r / MAX(1, mRows - 1) - 0.5f) * aspect;
               float uz = (lum - 0.5f) * depth;
               //audio jitter on the loud transients
               if (high > 0.01f)
                  uz += (Hash(c, r) - 0.5f) * high * 0.15f;

               //rotate X
               float y1 = uy * crx - uz * srx, z1 = uy * srx + uz * crx, x1 = ux;
               //rotate Y
               float x2 = x1 * cry + z1 * sry, z2 = -x1 * sry + z1 * cry, y2 = y1;
               //rotate Z
               float x3 = x2 * crz - y2 * srz, y3 = x2 * srz + y2 * crz, z3 = z2;

               float persp = focal / (focal - z3);
               float sx = cx + x3 * scale * persp;
               float sy = cy + y3 * scale * persp;

               float rr, gg, bb;
               CellColor(idx, lum, rr, gg, bb);
               float shade = ofClamp(0.55f + z3 * 0.6f, 0.25f, 1.0f); //nearer = brighter
               float ps = MAX(1.0f, ptBase * persp * (0.5f + lum));
               ofSetColor(rr * 255 * shade, gg * 255 * shade, bb * 255 * shade, 235);
               ofRect(sx - ps * 0.5f, sy - ps * 0.5f, ps, ps);
            }
         }
         ofPopStyle();
      }

      //film grain on top (cheap, no feedback buffer)
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
      DrawTextNormal("drop an image here", viewX + viewW * 0.5f - 60, viewY + viewH * 0.5f, 13);
      ofPopStyle();
   }

   //auto-tumble (unless the user is dragging)
   if (mEnabled && !mDragging)
   {
      mRotY += (0.004f + amp * 0.02f) * mSpinSpeed;
      if (mMode == kMode_Cloud)
         mRotX += (0.001f + amp * 0.006f) * mSpinSpeed;
   }

   mModeSelector->Draw();
   mDensitySlider->Draw();
   mGlitchSlider->Draw();
   mDepthSlider->Draw();
   mSpinSlider->Draw();
   mSensitivitySlider->Draw();
   mGrainSlider->Draw();
   mHueShiftSlider->Draw();
   mColorModeSelector->Draw();
   mPaletteSelector->Draw();
}

void PixelCloud::FilesDropped(std::vector<std::string> files, int x, int y)
{
   for (const std::string& f : files)
   {
      if (LoadImageFile(f))
         break; //load the first valid image
   }
}

void PixelCloud::OnClicked(float x, float y, bool right)
{
   //start a rotate-drag if the click lands in the view area (right of the control strip)
   if (!right && x > 112)
   {
      mDragging = true;
      mLastDragX = TheSynth->GetMouseX(GetOwningContainer());
      mLastDragY = TheSynth->GetMouseY(GetOwningContainer());
      return;
   }
   IDrawableModule::OnClicked(x, y, right);
}

void PixelCloud::Poll()
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

void PixelCloud::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mDensitySlider)
      RebuildGrid();
}

void PixelCloud::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   //image path is stored as raw json (kept out of the save-data system so SaveLayout can't be clobbered)
   if (moduleInfo.isMember("imagepath"))
      mImagePath = moduleInfo["imagepath"].asString();
   SetUpFromSaveData();
}

void PixelCloud::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["imagepath"] = mImagePath;
}

void PixelCloud::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   if (!mImagePath.empty())
      LoadImageFile(mImagePath);
}

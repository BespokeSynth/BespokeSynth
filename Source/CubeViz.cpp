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
//  CubeViz.cpp
//  Bespoke
//

#include "CubeViz.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace
{
   struct Quad
   {
      float x[4], y[4];
      float depth;
      float colorT; //palette position for this quad (spreads the palette across the surface)
   };
}

CubeViz::CubeViz()
: IAudioProcessor(gBufferSize)
{
   mWidth = 320;
   mHeight = 380;
   for (int i = 0; i < kMaxCells; ++i)
      mShapeGrid[i] = i % 14; //seeded; replaced when the user hits randomize
}

CubeViz::~CubeViz()
{
}

void CubeViz::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mSensitivitySlider = new FloatSlider(this, "sensitivity", 3, y, 100, 14, &mSensitivity, 0.1f, 6.0f);
   y += 17;
   mSpinSlider = new FloatSlider(this, "spin", 3, y, 100, 14, &mSpinSpeed, 0.0f, 3.0f);
   y += 17;
   mSeparationSlider = new FloatSlider(this, "separation", 3, y, 100, 14, &mSeparation, 0.0f, 3.0f);
   y += 17;
   mShapeSelector = new DropdownList(this, "shape", 3, y, &mShape, 100);
   y += 17;
   mCountXSlider = new IntSlider(this, "cols", 3, y, 100, 14, &mCountX, 1, 10);
   y += 17;
   mCountYSlider = new IntSlider(this, "rows", 3, y, 100, 14, &mCountY, 1, 10);
   y += 17;
   mRandomizeButton = new ClickButton(this, "randomize", 3, y);
   y += 17;
   mResetButton = new ClickButton(this, "reset", 3, y);
   y += 17;
   mSymmetryCheckbox = new Checkbox(this, "symmetry", 3, y, &mSymmetry);
   y += 17;
   mGrainSlider = new FloatSlider(this, "grain", 3, y, 100, 14, &mGrain, 0.0f, 0.5f);
   y += 17;
   mHueShiftSlider = new FloatSlider(this, "hue", 3, y, 100, 14, &mHueShift, 0.0f, 1.0f);
   y += 17;
   mPaletteSelector = new DropdownList(this, "palette", 3, y, &mPaletteIndex, 100);

   mShapeSelector->AddLabel("cube", 0);
   mShapeSelector->AddLabel("sphere", 1);
   mShapeSelector->AddLabel("ellipsoid", 2);
   mShapeSelector->AddLabel("cone", 3);
   mShapeSelector->AddLabel("cylinder", 4);
   mShapeSelector->AddLabel("torus", 5);
   mShapeSelector->AddLabel("paraboloid", 6);
   mShapeSelector->AddLabel("hyperboloid", 7);
   mShapeSelector->AddLabel("elliptic torus", 8);
   mShapeSelector->AddLabel("figure-8", 9);
   mShapeSelector->AddLabel("bohemian dome", 10);
   mShapeSelector->AddLabel("tritorus", 11);
   mShapeSelector->AddLabel("limpet torus", 12);
   mShapeSelector->AddLabel("bow tie", 13);

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);
}

void CubeViz::PaletteColor(float t, float& rOut, float& gOut, float& bOut) const
{
   VizPaletteColor(mPaletteIndex, t, mHueShift, rOut, gOut, bOut);
}

void CubeViz::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizeButton)
   {
      for (int i = 0; i < kMaxCells; ++i)
         mShapeGrid[i] = (int)ofRandom(0, 14);
      mRandomized = true;
   }
   else if (button == mResetButton)
   {
      //back to a uniform grid of the shape chosen in the dropdown
      mRandomized = false;
   }
}

//parametric surface library. u01,v01 in [0,1]; returns a point roughly within [-1,1]^3.
void CubeViz::SurfacePoint(int shape, float u01, float v01, float& x, float& y, float& z) const
{
   const float TAU = FTWO_PI;
   const float RT2 = 1.41421356f;
   switch (shape)
   {
      case 1: //sphere
      {
         float u = u01 * TAU, v = v01 * PI;
         x = sinf(v) * cosf(u);
         y = cosf(v);
         z = sinf(v) * sinf(u);
         break;
      }
      case 2: //ellipsoid
      {
         float u = u01 * TAU, v = v01 * PI;
         x = 0.72f * sinf(v) * cosf(u);
         y = 1.0f * cosf(v);
         z = 0.5f * sinf(v) * sinf(u);
         break;
      }
      case 3: //cone
      {
         float u = u01 * TAU, v = v01;
         float r = (1.0f - v) * 0.85f;
         x = r * cosf(u);
         y = (v - 0.5f) * 1.7f;
         z = r * sinf(u);
         break;
      }
      case 4: //cylinder
      {
         float u = u01 * TAU, v = v01;
         x = 0.7f * cosf(u);
         y = (v - 0.5f) * 1.8f;
         z = 0.7f * sinf(u);
         break;
      }
      case 5: //torus
      {
         float u = u01 * TAU, v = v01 * TAU;
         float R = 0.62f, r = 0.30f;
         x = (R + r * cosf(v)) * cosf(u);
         y = r * sinf(v);
         z = (R + r * cosf(v)) * sinf(u);
         break;
      }
      case 6: //paraboloid
      {
         float u = u01 * TAU, v = v01;
         float r = v * 0.95f;
         x = r * cosf(u);
         y = (v * v) * 1.7f - 0.85f;
         z = r * sinf(u);
         break;
      }
      case 7: //hyperboloid (one sheet)
      {
         float u = u01 * TAU, v = (v01 * 2.0f - 1.0f);
         float r = sqrtf(1.0f + v * v) * 0.5f;
         x = r * cosf(u);
         y = v * 0.9f;
         z = r * sinf(u);
         break;
      }
      case 8: //elliptic torus (Bourke)
      {
         float u = (u01 * 2.0f - 1.0f) * PI, v = (v01 * 2.0f - 1.0f) * PI;
         float c = 1.3f, s = 0.42f;
         x = (c + cosf(v)) * cosf(u) * s;
         y = (sinf(v) + cosf(v)) * s;
         z = (c + cosf(v)) * sinf(u) * s;
         break;
      }
      case 9: //figure-8 torus (Bourke)
      {
         float u = (u01 * 2.0f - 1.0f) * PI, v = (v01 * 2.0f - 1.0f) * PI;
         float c = 1.0f, s = 0.5f;
         float common = c + sinf(v) * cosf(u) - sinf(2 * v) * sinf(u) / 2;
         float X = cosf(u) * common;
         float Y = sinf(u) * common;
         float Z = sinf(u) * sinf(v) + cosf(u) * sinf(2 * v) / 2;
         x = X * s;
         y = Z * s * 1.4f; //lift the thin axis so it reads in 3D
         z = Y * s;
         break;
      }
      case 10: //bohemian dome (Bourke)
      {
         float u = u01 * TAU, v = v01 * TAU;
         float a = 0.5f, b = 0.5f, cc = 0.9f, s = 0.95f;
         x = a * cosf(u) * s;
         z = (a * sinf(u) + b * cosf(v)) * s;
         y = cc * sinf(v) * s;
         break;
      }
      case 11: //triaxial tritorus (Bourke)
      {
         float u = (u01 * 2.0f - 1.0f) * PI, v = (v01 * 2.0f - 1.0f) * PI;
         float s = 0.42f, t = TAU / 3.0f;
         x = sinf(u) * (1 + cosf(v)) * s;
         y = sinf(u + t) * (1 + cosf(v + t)) * s;
         z = sinf(u + 2 * t) * (1 + cosf(v + 2 * t)) * s;
         break;
      }
      case 12: //limpet torus (Bourke)
      {
         float u = u01 * TAU, v = v01 * TAU;
         float s = 0.7f;
         x = cosf(u) / (RT2 + sinf(v)) * s;
         z = sinf(u) / (RT2 + sinf(v)) * s;
         y = 1.0f / (RT2 + cosf(v)) * s - 0.5f;
         break;
      }
      case 13: //bow tie (Bourke)
      {
         float u = u01 * TAU, v = v01 * TAU;
         float s = 1.0f;
         x = sinf(u) / (RT2 + cosf(v)) * s;
         z = sinf(u) / (RT2 + sinf(v)) * s;
         y = cosf(u) / (1.0f + RT2) * s;
         break;
      }
      default: //sphere fallback
      {
         float u = u01 * TAU, v = v01 * PI;
         x = sinf(v) * cosf(u);
         y = cosf(v);
         z = sinf(v) * sinf(u);
         break;
      }
   }
}

void CubeViz::Process(double time)
{
   PROFILER(CubeViz);

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

void CubeViz::DrawShape(float cx, float cy, float halfW, float halfH, float rx, float ry, float rz, int shape, float paletteT, int gridN, bool mirror)
{
   const float cxx = cosf(rx), sxx = sinf(rx);
   const float cyy = cosf(ry), syy = sinf(ry);
   const float czz = cosf(rz), szz = sinf(rz);

   auto project = [&](float px, float py, float pz, float& ox, float& oy, float& oz)
   {
      float lx = px * halfW, ly = py * halfH, lz = pz * halfW;
      float y1 = ly * cxx - lz * sxx, z1 = ly * sxx + lz * cxx, x1 = lx;
      float x2 = x1 * cyy + z1 * syy, z2 = -x1 * syy + z1 * cyy, y2 = y1;
      float x3 = x2 * czz - y2 * szz, y3 = x2 * szz + y2 * czz, z3 = z2;
      ox = cx + (mirror ? -x3 : x3);
      oy = cy + y3;
      oz = z3;
   };

   std::vector<Quad> quads;

   auto addQuad = [&](float colorT,
                      float p0x, float p0y, float p0z, float p1x, float p1y, float p1z,
                      float p2x, float p2y, float p2z, float p3x, float p3y, float p3z)
   {
      Quad q;
      float oz;
      q.depth = 0;
      project(p0x, p0y, p0z, q.x[0], q.y[0], oz);
      q.depth += oz;
      project(p1x, p1y, p1z, q.x[1], q.y[1], oz);
      q.depth += oz;
      project(p2x, p2y, p2z, q.x[2], q.y[2], oz);
      q.depth += oz;
      project(p3x, p3y, p3z, q.x[3], q.y[3], oz);
      q.depth += oz;
      q.depth *= 0.25f;
      q.colorT = colorT;
      quads.push_back(q);
   };

   //colorT spans 0..1 across the whole surface so the selected palette is spread over each shape
   if (shape == 0)
   {
      //cube: 6 flat faces subdivided into a grid
      int g = MAX(2, gridN / 2);
      for (int face = 0; face < 6; ++face)
      {
         int axis = face / 2;
         float sign = (face & 1) ? 1.0f : -1.0f;
         for (int i = 0; i < g; ++i)
         {
            for (int j = 0; j < g; ++j)
            {
               float u0 = -1.0f + 2.0f * i / g, u1 = -1.0f + 2.0f * (i + 1) / g;
               float v0 = -1.0f + 2.0f * j / g, v1 = -1.0f + 2.0f * (j + 1) / g;
               float cu[4] = { u0, u1, u1, u0 };
               float cv[4] = { v0, v0, v1, v1 };
               float px[4], py[4], pz[4];
               for (int c = 0; c < 4; ++c)
               {
                  if (axis == 0)
                  {
                     px[c] = sign;
                     py[c] = cu[c];
                     pz[c] = cv[c];
                  }
                  else if (axis == 1)
                  {
                     px[c] = cu[c];
                     py[c] = sign;
                     pz[c] = cv[c];
                  }
                  else
                  {
                     px[c] = cu[c];
                     py[c] = cv[c];
                     pz[c] = sign;
                  }
               }
               float colorT = (face + (i + j) / (2.0f * g)) / 6.0f;
               addQuad(colorT, px[0], py[0], pz[0], px[1], py[1], pz[1], px[2], py[2], pz[2], px[3], py[3], pz[3]);
            }
         }
      }
   }
   else
   {
      for (int i = 0; i < gridN; ++i)
      {
         for (int j = 0; j < gridN; ++j)
         {
            float u0 = (float)i / gridN, u1 = (float)(i + 1) / gridN;
            float v0 = (float)j / gridN, v1 = (float)(j + 1) / gridN;
            float ax, ay, az, bx, by, bz, ccx, ccy, ccz, dx, dy, dz;
            SurfacePoint(shape, u0, v0, ax, ay, az);
            SurfacePoint(shape, u1, v0, bx, by, bz);
            SurfacePoint(shape, u1, v1, ccx, ccy, ccz);
            SurfacePoint(shape, u0, v1, dx, dy, dz);
            float colorT = 0.5f * ((float)i / gridN) + 0.5f * ((float)j / gridN);
            addQuad(colorT, ax, ay, az, bx, by, bz, ccx, ccy, ccz, dx, dy, dz);
         }
      }
   }

   std::sort(quads.begin(), quads.end(), [](const Quad& a, const Quad& b)
             {
                return a.depth < b.depth;
             });

   //depth-based diffuse light: nearer facets brighter, far ones darker -> real 3D shading (not flat white)
   float dmin = 1e18f, dmax = -1e18f;
   for (const Quad& q : quads)
   {
      dmin = MIN(dmin, q.depth);
      dmax = MAX(dmax, q.depth);
   }
   float drange = MAX(1e-3f, dmax - dmin);

   ofPushStyle();
   ofFill();
   for (const Quad& q : quads)
   {
      float shade = 0.35f + 0.65f * ((q.depth - dmin) / drange);
      float pr, pg, pb;
      PaletteColor(paletteT + q.colorT * 0.85f, pr, pg, pb); //sweep the palette across the surface
      ofSetColor(ofClamp(pr * shade, 0.0f, 1.0f) * 255, ofClamp(pg * shade, 0.0f, 1.0f) * 255, ofClamp(pb * shade, 0.0f, 1.0f) * 255, 255);
      ofBeginShape();
      ofVertex(q.x[0], q.y[0]);
      ofVertex(q.x[1], q.y[1]);
      ofVertex(q.x[2], q.y[2]);
      ofVertex(q.x[3], q.y[3]);
      ofEndShape(true);
   }
   ofPopStyle();
}

void CubeViz::DrawModule()
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
         ofSetColor(br * 255, bgc * 255, bb * 255, 9);
         ofCircle(cx, cy, MAX(w, h) * (0.4f + i * 0.15f));
      }
   }
   ofPopStyle();

   if (mEnabled)
   {
      float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);
      float baseT = ofClamp(mHighEnergy / (mAmplitude + 0.02f), 0.0f, 1.0f);

      mRotX += (0.003f + amp * 0.020f) * mSpinSpeed;
      mRotY += (0.005f + amp * 0.030f) * mSpinSpeed;
      mRotZ += (0.002f + amp * 0.015f) * mSpinSpeed;

      int nx = ofClamp(mCountX, 1, 10);
      int ny = ofClamp(mCountY, 1, 10);
      int total = nx * ny;

      //adaptive mesh density so the total quad budget stays bounded (keeps the frame rate high).
      //fewer shapes -> denser mesh -> smoother/less faceted surfaces.
      int draws = total * (mSymmetry ? 2 : 1);
      int quadsPerShape = MAX(64, 5200 / MAX(1, draws));
      int gridN = (int)ofClamp(sqrtf((float)quadsPerShape), 6.0f, 30.0f);

      float minDim = MIN(w, h);
      int maxCount = MAX(nx, ny);
      float size = minDim * (0.15f + amp * 0.06f) * (1.7f / sqrtf((float)maxCount + 1.0f));
      float gap = minDim * 0.02f * (0.5f + amp * 1.0f) * mSeparation;
      float stepX = 2 * size + gap;
      float stepY = 2 * size + gap;

      for (int iy = 0; iy < ny; ++iy)
      {
         for (int ix = 0; ix < nx; ++ix)
         {
            int cell = iy * nx + ix;
            int shape = mRandomized ? mShapeGrid[cell % kMaxCells] : mShape;
            float px = cx + (ix - (nx - 1) * 0.5f) * stepX;
            float py = cy + (iy - (ny - 1) * 0.5f) * stepY;
            float ph = cell * 0.35f;
            float wob = amp * 0.4f;
            float rx = mRotX + ph + sinf((float)(gTime * 0.0009) + cell) * wob;
            float ry = mRotY + ph + sinf((float)(gTime * 0.0011) + cell * 1.7f) * wob;
            float rz = mRotZ + ph * 0.5f;
            //spread the palette evenly across the whole grid, with a little audio drift
            float t = (float)cell / MAX(1, total) + baseT * 0.2f;
            DrawShape(px, py, size, size, rx, ry, rz, shape, t, gridN, false);
            if (mSymmetry)
               DrawShape(px, py, size, size, rx, ry, rz, shape, t, gridN, true);
         }
      }

      //film-grain noise overlay (cheap, on top - no feedback buffer)
      if (mGrain > 0.001f)
      {
         ofPushStyle();
         ofFill();
         int gn = (int)(mGrain * w * h * 0.02f);
         for (int i = 0; i < gn; ++i)
         {
            ofSetColor(255, 255, 255, ofRandom(6, 30));
            ofRect(ofRandom(0, w), ofRandom(0, h), 1, 1);
         }
         ofPopStyle();
      }
   }

   mSensitivitySlider->Draw();
   mSpinSlider->Draw();
   mSeparationSlider->Draw();
   mShapeSelector->Draw();
   mCountXSlider->Draw();
   mCountYSlider->Draw();
   mRandomizeButton->Draw();
   mResetButton->Draw();
   mSymmetryCheckbox->Draw();
   mGrainSlider->Draw();
   mHueShiftSlider->Draw();
   mPaletteSelector->Draw();
}

void CubeViz::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void CubeViz::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void CubeViz::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

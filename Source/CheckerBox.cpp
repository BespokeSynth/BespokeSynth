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
//  CheckerBox.cpp  (GPU port - same behavior as the original, rendered with a fragment shader)
//

#include "CheckerBox.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "VizPalettes.h"
#include "juce_opengl/juce_opengl.h"
#include <cmath>

using namespace juce::gl;

namespace
{
   const char* kFragSrc =
   "#version 150\n"
   "in vec2 vUv;\n"
   "out vec4 fragColor;\n"
   "uniform float uTime, uStep, uReact, uDistort, uChroma, uGrain, uExposure, uGrid, uHue;\n"
   "uniform int uPattern, uColorMode;\n"
   "uniform vec2 uRes;\n"
   "uniform vec3 uPalA, uPalB, uPalC, uPalD;\n"
   "float hash(vec2 p){ return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }\n"
   "vec3 palette(float t){ return clamp(uPalA + uPalB*cos(6.2831853*(uPalC*t + uPalD + uHue)), 0.0, 1.0); }\n"
   "vec3 cellColor(float m){\n"
   "  vec3 col;\n"
   "  if(uColorMode==0){ float v = (m<0.5)?1.0:0.06; col=vec3(v); }\n"
   "  else { col = palette((m<0.5)?0.15:0.65); }\n"
   "  return clamp(col*uExposure, 0.0, 1.0);\n"
   "}\n"
   "vec3 checker(vec2 uv){\n"
   "  float t = uTime*0.001;\n"
   "  float i, j;\n"
   "  if(uPattern<=1){\n" // grid / bend
   "    float d = uDistort*(0.7+uReact);\n"
   "    float bend = (uPattern==1)? d : d*0.25;\n"
   "    vec2 n = uv-0.5;\n"
   "    float r2 = dot(n,n);\n"
   "    float warp = 1.0 + bend*(0.9*r2*4.0)*(0.6+0.4*sin(t*2.0));\n"
   "    vec2 wn = n*warp + bend*0.06*vec2(sin(t*3.0+n.y*10.0), sin(t*2.5+n.x*10.0));\n"
   "    vec2 cell = (wn+0.5)*uGrid;\n"
   "    i=floor(cell.x); j=floor(cell.y);\n"
   "  } else {\n" // spiral / tunnel
   "    float d = uDistort*(0.7+uReact);\n"
   "    vec2 n = uv-0.5; n.x *= uRes.x/max(1.0,uRes.y);\n"
   "    float rr = clamp(length(n)*2.0, 0.0, 1.0);\n"
   "    float ang = atan(n.y, n.x);\n"
   "    float rings = uGrid;\n"
   "    float sectors = max(6.0, uGrid);\n"
   "    float twist = d*3.0;\n"
   "    float rot = (uPattern==3)? t*(1.0+uReact)*1.5 : t*0.2;\n"
   "    float f = (uPattern==3)? pow(rr, 1.0/2.2) : rr;\n"
   "    i = floor(f*rings);\n"
   "    float aa = ang - twist*f - rot;\n"
   "    j = floor((aa/6.2831853)*sectors);\n"
   "  }\n"
   "  float m = mod(i+j+uStep, 2.0);\n"
   "  return cellColor(m);\n"
   "}\n"
   "void main(){\n"
   "  vec2 uv = vUv;\n"
   "  vec3 col;\n"
   "  if(uChroma>0.01){\n"
   "    float dx = uChroma*(1.0/max(2.0,uGrid))*0.9 + uReact*0.02;\n"
   "    col = vec3(checker(uv+vec2(dx,0.0)).r, checker(uv).g, checker(uv-vec2(dx,0.0)).b);\n"
   "  } else { col = checker(uv); }\n"
   "  col += (hash(vUv*uRes + uTime*0.001)*2.0-1.0)*uGrain;\n"
   "  fragColor = vec4(clamp(col, 0.0, 1.0), 1.0);\n"
   "}\n";
}

CheckerBox::CheckerBox()
: IAudioProcessor(gBufferSize)
{
   mWidth = 360;
   mHeight = 300;
}

CheckerBox::~CheckerBox()
{
   VizGL::DestroyFbo(mOut);
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

   y += 17;
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

bool CheckerBox::EnsureShader()
{
   if (mShaderTried)
      return mProgram != 0;
   mShaderTried = true;

   mProgram = VizGL::CompileProgram(kFragSrc);
   if (mProgram == 0)
      return false;

   mLocTime = glGetUniformLocation(mProgram, "uTime");
   mLocStep = glGetUniformLocation(mProgram, "uStep");
   mLocReact = glGetUniformLocation(mProgram, "uReact");
   mLocDistort = glGetUniformLocation(mProgram, "uDistort");
   mLocPattern = glGetUniformLocation(mProgram, "uPattern");
   mLocChroma = glGetUniformLocation(mProgram, "uChroma");
   mLocGrain = glGetUniformLocation(mProgram, "uGrain");
   mLocExposure = glGetUniformLocation(mProgram, "uExposure");
   mLocColorMode = glGetUniformLocation(mProgram, "uColorMode");
   mLocGrid = glGetUniformLocation(mProgram, "uGrid");
   mLocRes = glGetUniformLocation(mProgram, "uRes");
   mLocHue = glGetUniformLocation(mProgram, "uHue");
   mLocPalA = glGetUniformLocation(mProgram, "uPalA");
   mLocPalB = glGetUniformLocation(mProgram, "uPalB");
   mLocPalC = glGetUniformLocation(mProgram, "uPalC");
   mLocPalD = glGetUniformLocation(mProgram, "uPalD");
   return true;
}

void CheckerBox::Cook()
{
   if (!EnsureShader())
      return;
   if (!VizGL::EnsureFbo(mOut, mResW, mResH))
      return;

   float amp = ofClamp(mAmplitude * mSensitivity * 6.0f, 0.0f, 1.0f);
   float step = floorf((float)(gTime * 0.001) * mSpeed * 4.0f * (1.0f + amp));

   int pi = mPaletteIndex;
   if (pi < 0)
      pi = 0;
   if (pi >= kNumVizPalettes)
      pi = kNumVizPalettes - 1;
   const VizPal& pal = kVizPalettes[pi];

   float t = (float)gTime;
   float distort = mDistort, chroma = mChroma, grain = mGrain, expo = mExposure, grid = (float)mGridN, hue = mHueShift;
   int pattern = mPattern, colorMode = mColorMode;
   float rw = (float)mOut.w, rh = (float)mOut.h;

   VizGL::RunShaderPass(mOut, mProgram, [=]()
                        {
                           if (mLocTime >= 0)
                              glUniform1f(mLocTime, t);
                           if (mLocStep >= 0)
                              glUniform1f(mLocStep, step);
                           if (mLocReact >= 0)
                              glUniform1f(mLocReact, amp);
                           if (mLocDistort >= 0)
                              glUniform1f(mLocDistort, distort);
                           if (mLocChroma >= 0)
                              glUniform1f(mLocChroma, chroma);
                           if (mLocGrain >= 0)
                              glUniform1f(mLocGrain, grain);
                           if (mLocExposure >= 0)
                              glUniform1f(mLocExposure, expo);
                           if (mLocGrid >= 0)
                              glUniform1f(mLocGrid, grid);
                           if (mLocHue >= 0)
                              glUniform1f(mLocHue, hue);
                           if (mLocPattern >= 0)
                              glUniform1i(mLocPattern, pattern);
                           if (mLocColorMode >= 0)
                              glUniform1i(mLocColorMode, colorMode);
                           if (mLocRes >= 0)
                              glUniform2f(mLocRes, rw, rh);
                           if (mLocPalA >= 0)
                              glUniform3f(mLocPalA, pal.a[0], pal.a[1], pal.a[2]);
                           if (mLocPalB >= 0)
                              glUniform3f(mLocPalB, pal.b[0], pal.b[1], pal.b[2]);
                           if (mLocPalC >= 0)
                              glUniform3f(mLocPalC, pal.c[0], pal.c[1], pal.c[2]);
                           if (mLocPalD >= 0)
                              glUniform3f(mLocPalD, pal.d[0], pal.d[1], pal.d[2]);
                        });
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
   int rw = MAX(1080, ((int)viewW & ~1));
   int rh = MAX(1080, ((int)viewH & ~1));

   mResW = rw;
   mResH = rh;

   if (EnsureShader())
   {
      Cook();
      VizGL::DrawTexture(GetOutputTexture(), viewX, viewY, viewW, viewH);
   }
   else
   {
      ofPushStyle();
      ofSetColor(240, 140, 120);
      DrawTextNormal("shader unavailable (see console)", (int)viewX + 6, (int)viewY + 18, 11);
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

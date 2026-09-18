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
//  VideoCloud.cpp
//  Bespoke
//

#include "VideoCloud.h"
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
#include <cstdlib>

namespace
{
   const int kMaxCells = 12000; //draw ceiling per frame
   const int kMaxFrames = 300; //frame-count ceiling
   const int kExtractW = 180; //ffmpeg downscale width
   const float kExtractFps = 15; //ffmpeg sampling rate

   inline float Hash(int x, int y)
   {
      int h = x * 374761393 + y * 668265263;
      h = (h ^ (h >> 13)) * 1274126177;
      return ((h ^ (h >> 16)) & 0x7fffffff) / (float)0x7fffffff;
   }
   inline float Lum(float r, float g, float b) { return 0.299f * r + 0.587f * g + 0.114f * b; }
}

VideoCloud::VideoCloud()
: IAudioProcessor(gBufferSize)
{
   mWidth = 470;
   mHeight = 340;
}

VideoCloud::~VideoCloud()
{
   VizGL::DestroyFbo(mOutputFbo);
   if (mLoaderThread.joinable())
      mLoaderThread.join();
   TheTransport->RemoveListener(this);
}

void VideoCloud::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   const float x = 3, wdt = 100;
   mDensitySlider = new IntSlider(this, "density", x, y, wdt, 14, &mDensity, 16, 160);
   y += 15;
   mFpsSlider = new FloatSlider(this, "fps", x, y, wdt, 14, &mFps, 1.0f, 30.0f);
   y += 15;
   mSyncCheckbox = new Checkbox(this, "sync", x, y, &mSync);
   y += 15;
   mIntervalSelector = new DropdownList(this, "rate", x, y, (int*)(&mInterval), wdt);
   y += 15;
   mLoopCheckbox = new Checkbox(this, "loop", x, y, &mLoop);
   y += 15;
   mMaskSelector = new DropdownList(this, "mask", x, y, &mMaskMode, wdt);
   y += 15;
   mThresholdSlider = new FloatSlider(this, "threshold", x, y, wdt, 14, &mThreshold, 0.0f, 1.0f);
   y += 15;
   mSymmetrySlider = new IntSlider(this, "symmetry", x, y, wdt, 14, &mSymmetry, 1, 4);
   y += 15;
   mKaleidoSlider = new IntSlider(this, "kaleido", x, y, wdt, 14, &mKaleido, 1, 12);
   y += 15;
   mDistortSlider = new FloatSlider(this, "distort", x, y, wdt, 14, &mDistort, 0.0f, 1.0f);
   y += 15;
   mGlitchSlider = new FloatSlider(this, "glitch", x, y, wdt, 14, &mGlitch, 0.0f, 1.0f);
   y += 15;
   mChromaSlider = new FloatSlider(this, "chroma", x, y, wdt, 14, &mChroma, 0.0f, 1.0f);
   y += 15;
   mReactSlider = new FloatSlider(this, "react", x, y, wdt, 14, &mReact, 0.0f, 2.0f);
   y += 15;
   mExposureSlider = new FloatSlider(this, "exposure", x, y, wdt, 14, &mExposure, 0.0f, 3.0f);
   y += 15;
   mGrainSlider = new FloatSlider(this, "grain", x, y, wdt, 14, &mGrain, 0.0f, 0.3f);
   y += 15;
   mPosterizeSlider = new IntSlider(this, "oil", x, y, wdt, 14, &mPosterize, 0, 8);
   y += 15;
   mEdgesSlider = new FloatSlider(this, "comic", x, y, wdt, 14, &mEdges, 0.0f, 1.0f);
   y += 15;
   mHalftoneCheckbox = new Checkbox(this, "halftone", x, y, &mHalftone);
   y += 15;
   mColorModeSelector = new DropdownList(this, "color", x, y, &mColorMode, wdt);
   y += 15;
   mHueShiftSlider = new FloatSlider(this, "hue", x, y, wdt, 14, &mHueShift, 0.0f, 1.0f);
   y += 15;
   mPaletteSelector = new DropdownList(this, "palette", x, y, &mPaletteIndex, wdt);

   mMaskSelector->AddLabel("mask off", 0);
   mMaskSelector->AddLabel("motion", 1);
   mMaskSelector->AddLabel("luma", 2);

   mColorModeSelector->AddLabel("video", 0);
   mColorModeSelector->AddLabel("palette", 1);
   mColorModeSelector->AddLabel("mono", 2);

   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);

   for (int i = 0; i < kNumVizPalettes; ++i)
      mPaletteSelector->AddLabel(kVizPaletteNames[i], i);

   y += 15;
}

void VideoCloud::Init()
{
   //Transport::AddListener asserts the module is initialized, so add the listener in Init() (after
   //IDrawableModule::Init sets that flag), not in CreateUIControls
   IDrawableModule::Init();
   UpdateTransportListener();
}

void VideoCloud::UpdateTransportListener()
{
   TransportListenerInfo* info = TheTransport->GetListenerInfo(this);
   if (info != nullptr)
      info->mInterval = mInterval;
   else
      TheTransport->AddListener(this, mInterval, OffsetInfo(0, false), false);
}

void VideoCloud::PaletteColor(float t, float& rOut, float& gOut, float& bOut) const
{
   VizPaletteColor(mPaletteIndex, t, mHueShift, rOut, gOut, bOut);
}

std::string VideoCloud::FindFfmpeg() const
{
   const char* candidates[] = { "/opt/homebrew/bin/ffmpeg", "/usr/local/bin/ffmpeg", "/usr/bin/ffmpeg" };
   for (const char* c : candidates)
   {
      if (juce::File(c).existsAsFile())
         return c;
   }
   return "ffmpeg"; //fall back to PATH
}

void VideoCloud::FilesDropped(std::vector<std::string> files, int x, int y)
{
   if (mLoading) //already decoding one; ignore extra drops
      return;
   for (const std::string& f : files)
   {
      juce::String lower = juce::String(f).toLowerCase();
      if (lower.endsWith(".mp4") || lower.endsWith(".mov") || lower.endsWith(".gif") || lower.endsWith(".m4v") || lower.endsWith(".webm"))
      {
         if (mLoaderThread.joinable())
            mLoaderThread.join();
         mLoading = true;
         mStatus = "decoding...";
         mLoaderThread = std::thread(&VideoCloud::LoadWorker, this, f); //decode off the UI thread
         break;
      }
   }
}

void VideoCloud::LoadWorker(std::string path)
{
   std::string ffmpeg = FindFfmpeg();

   juce::File outDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("bespoke_video_" + juce::String(juce::Random::getSystemRandom().nextInt(1000000)));
   outDir.createDirectory();

   //extract frames with ChildProcess (args passed as an array -> no shell, so no injection risk)
   juce::ChildProcess proc;
   juce::StringArray args;
   args.add(juce::String(ffmpeg));
   args.add("-y");
   args.add("-i");
   args.add(juce::String(path));
   args.add("-vf");
   args.add("fps=" + juce::String((int)kExtractFps) + ",scale=" + juce::String(kExtractW) + ":-1");
   args.add("-frames:v");
   args.add(juce::String(kMaxFrames));
   args.add(outDir.getChildFile("f%04d.png").getFullPathName());
   bool started = proc.start(args);
   if (started)
      proc.waitForProcessToFinish(30000);

   //decode into local buffers (this thread only), then swap into the module under a lock
   int srcW = 0, srcH = 0;
   std::vector<std::vector<unsigned char>> newFrames;
   for (int i = 1; i <= kMaxFrames; ++i)
   {
      juce::File frameFile = outDir.getChildFile(juce::String::formatted("f%04d.png", i));
      if (!frameFile.existsAsFile())
         break;
      juce::Image img = juce::ImageFileFormat::loadFrom(frameFile);
      if (!img.isValid())
         break;
      if (srcW == 0)
      {
         srcW = img.getWidth();
         srcH = img.getHeight();
      }
      std::vector<unsigned char> rgb((size_t)srcW * srcH * 3, 0);
      juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
      for (int py = 0; py < srcH; ++py)
      {
         for (int px = 0; px < srcW; ++px)
         {
            juce::Colour col = bmp.getPixelColour(MIN(px, img.getWidth() - 1), MIN(py, img.getHeight() - 1));
            size_t o = ((size_t)py * srcW + px) * 3;
            rgb[o + 0] = col.getRed();
            rgb[o + 1] = col.getGreen();
            rgb[o + 2] = col.getBlue();
         }
      }
      newFrames.push_back(std::move(rgb));
   }
   outDir.deleteRecursively();

   std::vector<unsigned char> newBg;
   BuildBackground(newFrames, srcW, srcH, newBg);

   {
      std::lock_guard<std::mutex> lock(mFramesMutex);
      if (newFrames.empty())
      {
         mStatus = !started ? "ffmpeg not found - run: brew install ffmpeg" : "couldn't read that video";
      }
      else
      {
         mFrames = std::move(newFrames);
         mBackground = std::move(newBg);
         mSrcW = srcW;
         mSrcH = srcH;
         mStartTime = gTime;
         mSyncFrame = 0;
         mStatus = std::to_string((int)mFrames.size()) + " frames";
      }
   }
   mLoading = false;
}

void VideoCloud::BuildBackground(const std::vector<std::vector<unsigned char>>& frames, int w, int hh, std::vector<unsigned char>& outBg) const
{
   //temporal average -> a rough "empty scene" the moving subject is measured against
   outBg.assign((size_t)w * hh * 3, 0);
   if (frames.empty())
      return;
   std::vector<float> accum((size_t)w * hh * 3, 0.0f);
   for (const auto& fr : frames)
      for (size_t i = 0; i < accum.size() && i < fr.size(); ++i)
         accum[i] += fr[i];
   float inv = 1.0f / (float)frames.size();
   for (size_t i = 0; i < accum.size(); ++i)
      outBg[i] = (unsigned char)ofClamp(accum[i] * inv, 0.0f, 255.0f);
}

void VideoCloud::OnTimeEvent(double time)
{
   if (!mSync || mFrames.empty())
      return;
   int n = (int)mFrames.size();
   mSyncFrame = mLoop ? (mSyncFrame + 1) % n : MIN(mSyncFrame + 1, n - 1);
}

void VideoCloud::Process(double time)
{
   PROFILER(VideoCloud);
   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   float sumSq = 0;
   float* ch0 = GetBuffer()->GetChannel(0);
   for (int i = 0; i < bufferSize; ++i)
      sumSq += ch0[i] * ch0[i];
   mAmplitude = mAmplitude * 0.85f + sqrtf(sumSq / MAX(1, bufferSize)) * 0.15f;

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

unsigned int VideoCloud::GetOutputTexture()
{
   return mOutputFbo.fb ? mOutputFbo.fb->texture : 0;
}

void VideoCloud::CookIfNeeded(int frameId)
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

   const float viewX = 116;
   const float viewY = 3;
   const float viewW = MAX(20.0f, w - viewX - 4);
   const float viewH = MAX(20.0f, h - viewY - 4);

   float amp = ofClamp(mAmplitude * 6.0f, 0.0f, 1.0f);

   //hold the frames lock while we read them, so the loader thread can't swap mid-draw
   std::lock_guard<std::mutex> framesLock(mFramesMutex);
   if (!mFrames.empty() && mSrcW > 0)
   {
      int n = (int)mFrames.size();
      int idx;
      if (mSync)
      {
         idx = ((mSyncFrame % n) + n) % n;
      }
      else
      {
         double elapsed = (gTime - mStartTime) * 0.001;
         double ff = elapsed * mFps;
         idx = mLoop ? (int)(((long long)ff) % n) : (int)MIN((double)(n - 1), MAX(0.0, ff));
      }
      const unsigned char* fr = mFrames[idx].data();
      const unsigned char* bg = mBackground.data();

      int cols = ofClamp(mDensity, 8, 200);
      int rows = MAX(1, (int)((float)cols * mSrcH / mSrcW));
      if ((long)cols * rows > kMaxCells)
      {
         float sc = sqrtf((float)kMaxCells / ((float)cols * rows));
         cols = MAX(8, (int)(cols * sc));
         rows = MAX(1, (int)(rows * sc));
      }
      float cell = MIN(viewW / cols, viewH / rows);
      float gridW = cell * cols, gridH = cell * rows;
      float ox = viewX + (viewW - gridW) * 0.5f;
      float oy = viewY + (viewH - gridH) * 0.5f;

      float t = (float)(gTime * 0.001);
      int frameBlock = (int)(gTime * 0.02);
      //audio drives glitch / distortion / brightness / kaleidoscope spin, scaled by "react"
      float glitch = ofClamp(mGlitch + amp * mReact * 0.6f, 0.0f, 1.0f);
      float distort = ofClamp(mDistort + amp * mReact * 0.5f, 0.0f, 1.5f);
      float exposureEff = mExposure * (1.0f + amp * mReact * 0.7f);
      float kaleidoRot = t * 0.25f + amp * mReact * 1.5f;
      float splitAmt = mChroma * cell * 1.6f + amp * (1.0f + mReact) * cell;

      ofPushStyle();
      ofFill();
      for (int r = 0; r < rows; ++r)
      {
         //per-row glitch tear
         float tear = 0.0f;
         if (glitch > 0.001f)
         {
            float rn = Hash(r, frameBlock);
            tear = (rn - 0.5f) * glitch * cell * 5.0f;
            if (rn > 0.92f - glitch * 0.2f)
               tear += (Hash(r, frameBlock + 7) - 0.5f) * gridW * 0.4f * glitch;
         }
         float cyc = oy + r * cell;

         for (int c = 0; c < cols; ++c)
         {
            float u = (c + 0.5f) / cols;
            float v = (r + 0.5f) / rows;
            //radial kaleidoscope: fold the angle into one mirrored wedge around the centre
            if (mKaleido > 1)
            {
               float dx = u - 0.5f, dy = v - 0.5f;
               float ang = atan2f(dy, dx) + kaleidoRot;
               float rr = sqrtf(dx * dx + dy * dy);
               float seg = FTWO_PI / mKaleido;
               ang = fmodf(ang, seg);
               if (ang < 0)
                  ang += seg;
               if (ang > seg * 0.5f)
                  ang = seg - ang; //mirror inside the wedge
               u = 0.5f + cosf(ang) * rr;
               v = 0.5f + sinf(ang) * rr;
            }
            //rectangular symmetry: fold each axis and rescale to use the full frame
            if (mSymmetry >= 2)
               u = (u < 0.5f) ? u * 2.0f : (1.0f - u) * 2.0f;
            if (mSymmetry >= 4)
               v = (v < 0.5f) ? v * 2.0f : (1.0f - v) * 2.0f;
            //distortion wobble (audio-reactive)
            if (distort > 0.001f)
            {
               u += distort * 0.08f * sinf(t * 2.0f + v * 10.0f);
               v += distort * 0.08f * sinf(t * 2.3f + u * 10.0f);
            }
            u = ofClamp(u, 0.0f, 1.0f);
            v = ofClamp(v, 0.0f, 1.0f);

            int px = ofClamp((int)(u * mSrcW), 0, mSrcW - 1);
            int py = ofClamp((int)(v * mSrcH), 0, mSrcH - 1);
            size_t o = ((size_t)py * mSrcW + px) * 3;
            float R = fr[o] / 255.0f, G = fr[o + 1] / 255.0f, B = fr[o + 2] / 255.0f;
            float lum = Lum(R, G, B);

            //auto subject mask
            if (mMaskMode == 1)
            {
               float bl = Lum(bg[o] / 255.0f, bg[o + 1] / 255.0f, bg[o + 2] / 255.0f);
               if (fabsf(lum - bl) < mThreshold)
                  continue;
            }
            else if (mMaskMode == 2)
            {
               if (lum < mThreshold)
                  continue;
            }

            //edges (comic outline): compare with the neighbour to the right + below
            bool isEdge = false;
            if (mEdges > 0.01f)
            {
               int px2 = MIN(mSrcW - 1, px + MAX(1, mSrcW / cols));
               int py2 = MIN(mSrcH - 1, py + MAX(1, mSrcH / rows));
               float lr = Lum(fr[((size_t)py * mSrcW + px2) * 3] / 255.0f, fr[((size_t)py * mSrcW + px2) * 3 + 1] / 255.0f, fr[((size_t)py * mSrcW + px2) * 3 + 2] / 255.0f);
               float ld = Lum(fr[((size_t)py2 * mSrcW + px) * 3] / 255.0f, fr[((size_t)py2 * mSrcW + px) * 3 + 1] / 255.0f, fr[((size_t)py2 * mSrcW + px) * 3 + 2] / 255.0f);
               if (fabsf(lum - lr) + fabsf(lum - ld) > (0.6f - mEdges * 0.5f))
                  isEdge = true;
            }

            //posterize (oil): quantise colours to a few levels
            if (mPosterize > 1)
            {
               float lv = (float)(mPosterize - 1);
               R = roundf(R * lv) / lv;
               G = roundf(G * lv) / lv;
               B = roundf(B * lv) / lv;
            }

            //colour mode
            if (mColorMode == 1)
               PaletteColor(lum, R, G, B);
            else if (mColorMode == 2)
               R = G = B = lum;

            if (isEdge)
               R = G = B = 0.0f; //comic ink outline
            else
               VizExpose(exposureEff, R, G, B); //brightness / exposure (audio-pulsed)

            float cx = ox + c * cell + tear;

            //chromatic aberration ghosts
            if (splitAmt > 0.5f && mColorMode != 2 && !isEdge)
            {
               ofSetColor(R * 255, 0, 0, 130);
               ofRect(cx - splitAmt * 0.5f, cyc, cell, cell);
               ofSetColor(0, 0, B * 255, 130);
               ofRect(cx + splitAmt * 0.5f, cyc, cell, cell);
            }

            ofSetColor(R * 255, G * 255, B * 255, 255);
            if (mHalftone)
            {
               float rad = cell * 0.5f * ofClamp(0.25f + lum, 0.0f, 1.0f);
               ofCircle(cx + cell * 0.5f, cyc + cell * 0.5f, rad);
            }
            else
            {
               ofRect(cx, cyc, cell + 0.5f, cell + 0.5f);
            }
         }
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
            ofSetColor(255, 255, 255, ofRandom(6, 24));
            ofRect(viewX + ofRandom(0, viewW), viewY + ofRandom(0, viewH), 1, 1);
         }
         ofPopStyle();
      }
   }
   else
   {
      ofPushStyle();
      ofSetColor(150, 155, 170, 170);
      DrawTextNormal(mStatus, viewX + 10, viewY + viewH * 0.5f, 13);
      ofPopStyle();
   }

   nvgEndFrame(gNanoVG);
   VizGL::UnbindFbo();
   glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
   glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
   gNanoVG = mainVG;
}

void VideoCloud::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   CookIfNeeded(0);

   VizGL::DrawTexture(mOutputFbo.fb ? mOutputFbo.fb->texture : 0, 0, 0, mWidth, mHeight);

   mIntervalSelector->SetShowing(mSync);
   mDensitySlider->Draw();
   mFpsSlider->Draw();
   mSyncCheckbox->Draw();
   mIntervalSelector->Draw();
   mLoopCheckbox->Draw();
   mMaskSelector->Draw();
   mThresholdSlider->Draw();
   mSymmetrySlider->Draw();
   mKaleidoSlider->Draw();
   mDistortSlider->Draw();
   mGlitchSlider->Draw();
   mChromaSlider->Draw();
   mReactSlider->Draw();
   mExposureSlider->Draw();
   mGrainSlider->Draw();
   mPosterizeSlider->Draw();
   mEdgesSlider->Draw();
   mHalftoneCheckbox->Draw();
   mColorModeSelector->Draw();
   mHueShiftSlider->Draw();
   mPaletteSelector->Draw();
}

void VideoCloud::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void VideoCloud::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void VideoCloud::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateTransportListener();
}

void VideoCloud::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void VideoCloud::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void VideoCloud::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

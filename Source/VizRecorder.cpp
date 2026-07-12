/**
    bespoke synth - shared visualizer video recorder (implementation)
**/

#include "VizRecorder.h"
#include "SynthGlobals.h"
#include "juce_opengl/juce_opengl.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_core/juce_core.h"

using namespace juce::gl; //JUCE exposes all GL entry points + enums under this namespace

#include "nanovg/nanovg.h"
//Bespoke compiles the entire GLES2 backend as `static` (TU-local), so we compile our own private
//copy of it here - same include recipe as ModularSynth.cpp. Being static, there is no duplicate
//symbol clash with ModularSynth's copy; each translation unit just gets its own.
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

VizRecorder::~VizRecorder()
{
   if (mRecording)
      Stop();
   //note: the FBO is intentionally left for the GL context teardown; deleting it needs a current
   //GL context which we don't necessarily have in the destructor.
}

std::string VizRecorder::FindFfmpeg() const
{
   const char* candidates[] = { "/opt/homebrew/bin/ffmpeg", "/usr/local/bin/ffmpeg", "/usr/bin/ffmpeg" };
   for (const char* c : candidates)
      if (juce::File(c).existsAsFile())
         return c;
   return "ffmpeg";
}

void VizRecorder::Toggle(int w, int h)
{
   if (!mRecording)
   {
      mW = MAX(16, w) & ~1; //even dimensions required by libx264 yuv420p
      mH = MAX(16, h) & ~1;
      mFrameCount = 0;
      mLastCaptureMs = -1000;
      juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("bespoke_rec_" + juce::String(juce::Random::getSystemRandom().nextInt(1000000)));
      dir.createDirectory();
      mTempDir = dir.getFullPathName().toStdString();
      mScratch.assign((size_t)mW * mH * 3, 0);
      mWriterRun = true;
      mWriter = std::thread(&VizRecorder::WriterLoop, this);
      mRecording = true;
      mStatus = "recording...";
   }
   else
   {
      Stop();
   }
}

void VizRecorder::CaptureFrame(const std::function<void()>& drawViz)
{
   if (!mRecording)
      return;

   //throttle to ~30fps so the encoded video plays at the right speed
   if (gTime - mLastCaptureMs < 33.0)
      return;
   mLastCaptureMs = gTime;

   //don't let the writer fall too far behind (drop frames instead of ballooning memory)
   {
      std::lock_guard<std::mutex> lk(mQueueMutex);
      if (mQueue.size() > 45)
         return;
   }

   //(re)create the framebuffer on the GL thread if needed
   NVGcontext* recCtx = gNanoVGRenderContexts[(int)NanoVGRenderContext::Screenshot];
   if (recCtx == nullptr)
      return;
   if (mFB == nullptr || mFBW != mW || mFBH != mH)
   {
      if (mFB != nullptr)
         nvgluDeleteFramebuffer(mFB);
      mFB = nvgluCreateFramebuffer(recCtx, mW, mH, 0);
      mFBW = mW;
      mFBH = mH;
      if (mFB == nullptr)
         return;
   }

   //save GL + nanovg state so the main pass is undisturbed
   GLint prevFBO = 0;
   GLint prevViewport[4] = { 0, 0, 0, 0 };
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
   glGetIntegerv(GL_VIEWPORT, prevViewport);
   NVGcontext* mainVG = gNanoVG;

   gNanoVG = recCtx;
   nvgluBindFramebuffer(mFB);
   glViewport(0, 0, mW, mH);
   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(gNanoVG, mW, mH, 1);

   drawViz(); //renders the visualization into [0,0,mW,mH]

   nvgEndFrame(gNanoVG);
   glFinish();
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, mW, mH, GL_RGB, GL_UNSIGNED_BYTE, mScratch.data());

   //restore
   glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFBO);
   glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
   gNanoVG = mainVG;

   //hand the frame to the writer thread
   {
      std::lock_guard<std::mutex> lk(mQueueMutex);
      mQueue.emplace_back(mFrameCount++, mScratch); //copy
   }
   mQueueCv.notify_one();
}

void VizRecorder::WriterLoop()
{
   while (true)
   {
      std::pair<int, std::vector<unsigned char>> item;
      {
         std::unique_lock<std::mutex> lk(mQueueMutex);
         mQueueCv.wait(lk, [&]
                       {
                          return !mQueue.empty() || !mWriterRun;
                       });
         if (mQueue.empty() && !mWriterRun)
            break;
         item = std::move(mQueue.front());
         mQueue.pop_front();
      }

      //GL pixels are bottom-up; flip into a top-down image and write PNG
      juce::Image img(juce::Image::RGB, mW, mH, false);
      {
         juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
         for (int y = 0; y < mH; ++y)
         {
            const unsigned char* src = &item.second[(size_t)(mH - 1 - y) * mW * 3];
            for (int x = 0; x < mW; ++x)
               bmp.setPixelColour(x, y, juce::Colour(src[x * 3], src[x * 3 + 1], src[x * 3 + 2]));
         }
      }
      juce::File f = juce::File(mTempDir).getChildFile(juce::String::formatted("f%05d.png", item.first));
      f.deleteFile();
      juce::FileOutputStream os(f);
      if (os.openedOk())
      {
         juce::PNGImageFormat png;
         png.writeImageToStream(img, os);
      }
   }
}

void VizRecorder::Stop()
{
   mRecording = false;
   //flush + join the writer
   mWriterRun = false;
   mQueueCv.notify_all();
   if (mWriter.joinable())
      mWriter.join();

   if (mFrameCount <= 0)
   {
      mStatus = "no frames captured";
      return;
   }

   //encode PNG sequence -> mp4 on the Desktop
   std::string ffmpeg = FindFfmpeg();
   juce::File outFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                        .getNonexistentChildFile("bespoke_viz", ".mp4", false);
   mOutPath = outFile.getFullPathName().toStdString();

   juce::ChildProcess proc;
   juce::StringArray args;
   args.add(juce::String(ffmpeg));
   args.add("-y");
   args.add("-framerate");
   args.add("30");
   args.add("-i");
   args.add(juce::File(mTempDir).getChildFile("f%05d.png").getFullPathName());
   args.add("-c:v");
   args.add("libx264");
   args.add("-pix_fmt");
   args.add("yuv420p");
   args.add(juce::String(mOutPath));
   bool started = proc.start(args);
   if (started)
      proc.waitForProcessToFinish(60000);

   juce::File(mTempDir).deleteRecursively();

   if (!started)
      mStatus = "ffmpeg not found - brew install ffmpeg";
   else if (juce::File(mOutPath).existsAsFile())
      mStatus = "saved to Desktop: " + juce::File(mOutPath).getFileName().toStdString();
   else
      mStatus = "encode failed";
}

/**
    bespoke synth - shared visualizer video recorder

    Records a visualizer's output to an mp4. It renders the viz into an offscreen NanoVG framebuffer
    (the same mechanism Bespoke's screenshot uses), reads the pixels back, hands them to a background
    thread that writes PNG frames, and on stop invokes ffmpeg to encode them to mp4 on the Desktop.
    Experimental / macOS-focused GL code. Requires ffmpeg on PATH (brew install ffmpeg).
**/

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

#include "IVisualNode.h"
#include "VizGL.h"

struct NVGLUframebuffer;

class VizRecorder
{
public:
   VizRecorder() { }
   ~VizRecorder();

   bool IsRecording() const { return mRecording; }
   std::string Status() const
   {
      std::lock_guard<std::mutex> lk(mQueueMutex);
      return mStatus;
   }

   //toggle recording on/off (call from a checkbox/button); w,h are the capture resolution
   void Toggle(int w, int h);

   //called every frame while recording: `drawViz` must render the visualization into [0,0,w,h]
   void CaptureFrame(const std::function<void()>& drawViz);

   //fast path: read pixels directly from an IVisualNode's GPU texture (no NanoVG re-render)
   void CaptureFrameFromTexture(IVisualNode* node, int outW, int outH);

private:
   void Stop(); //flush writer + encode with ffmpeg + cleanup
   void WriterLoop();
   std::string FindFfmpeg() const;

   std::atomic<bool> mRecording{ false };
   int mW{ 0 };
   int mH{ 0 };
   NVGLUframebuffer* mFB{ nullptr };
   int mFBW{ 0 };
   int mFBH{ 0 };
   std::vector<unsigned char> mScratch; //glReadPixels target (RGB)
   unsigned int mReadFBO{ 0 }; //GL FBO used to read back IVisualNode textures directly
   VizGL::Fbo mWriteFbo; //NanoVG FBO used as the target for the scale/crop shader pass
   unsigned int mCopyProgram{ 0 };
   int mFrameCount{ 0 };
   double mLastCaptureMs{ -1000 };
   std::string mTempDir;
   std::string mOutPath;
   std::string mStatus{ "" };

   //background PNG writer
   std::thread mWriter;
   mutable std::mutex mQueueMutex;
   std::condition_variable mQueueCv;
   std::deque<std::pair<int, std::vector<unsigned char>>> mQueue;
   std::atomic<bool> mWriterRun{ false };
};

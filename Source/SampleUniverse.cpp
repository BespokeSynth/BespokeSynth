#include "SampleUniverse.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FileStream.h"
#include "IAudioReceiver.h"
#include "UIControlMacros.h"

SampleUniverse::SampleUniverse()
: mOutputBuffer(gBufferSize)
{
   mSamples.reserve(100);
   mVoices.resize(32);
   for (auto& voice : mVoices)
   {
      voice.sampleIndex = -1;
      voice.playing = false;
      voice.playPos = 0.0;
   }
}

void SampleUniverse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   BUTTON(mPrevSeedButton, "<");
   BUTTON(mReseedButton, "*");
   BUTTON(mNextSeedButton, ">");
   UIBLOCK_NEWLINE();
   BUTTON(mClearButton, "clear samples");
   UIBLOCK_NEWLINE();
   CHECKBOX(mRecordCheckbox, "record path", &mRecording);
   UIBLOCK_NEWLINE();
   CHECKBOX(mPlayPathCheckbox, "play path", &mPlayingPath);
   UIBLOCK_NEWLINE();
   CHECKBOX(mLoopPathCheckbox, "loop path", &mLoopPath);
   UIBLOCK_NEWLINE();
   CHECKBOX(mReversePathCheckbox, "reverse", &mReversePath);
   UIBLOCK_NEWLINE();
   DROPDOWN(mQuantizeLengthSelector, "interval", (int*)(&mQuantizeInterval), 70);
   mQuantizeLengthSelector->AddLabel("32n", kInterval_32n);
   mQuantizeLengthSelector->AddLabel("16nt", kInterval_16nt);
   mQuantizeLengthSelector->AddLabel("16n", kInterval_16n);
   mQuantizeLengthSelector->AddLabel("8nt", kInterval_8nt);
   mQuantizeLengthSelector->AddLabel("8n", kInterval_8n);
   mQuantizeLengthSelector->AddLabel("4nt", kInterval_4nt);
   mQuantizeLengthSelector->AddLabel("4n", kInterval_4n);
   mQuantizeLengthSelector->AddLabel("2n", kInterval_2n);
   mQuantizeLengthSelector->AddLabel("1", kInterval_1n);
   mQuantizeLengthSelector->AddLabel("2", kInterval_2);
   mQuantizeLengthSelector->AddLabel("4", kInterval_4);
   mQuantizeLengthSelector->AddLabel("8", kInterval_8);
   mQuantizeLengthSelector->AddLabel("16", kInterval_16);
   DROPDOWN(mShapeSelector, "shape", (int*)(&mCurrentShape), 80);
   mShapeSelector->AddLabel("random", kShape_Random);
   mShapeSelector->AddLabel("line", kShape_Line);
   mShapeSelector->AddLabel("circle", kShape_Circle);
   mShapeSelector->AddLabel("spiral", kShape_Spiral);
   mShapeSelector->AddLabel("square", kShape_Square);
   mShapeSelector->AddLabel("triangle", kShape_Triangle);
   mShapeSelector->AddLabel("cross", kShape_Cross);
   mShapeSelector->AddLabel("star", kShape_Star);
   mShapeSelector->AddLabel("torus", kShape_Torus);
   mShapeSelector->AddLabel("cat", kShape_Cat);
   mShapeSelector->AddLabel("frog", kShape_Frog);
   mShapeSelector->AddLabel("computer", kShape_Computer);
   UIBLOCK_NEWLINE();

   mPrevSeedButton->PositionTo(mSeedEntry, kAnchor_Right);
   mReseedButton->PositionTo(mPrevSeedButton, kAnchor_Right);
   mNextSeedButton->PositionTo(mReseedButton, kAnchor_Right);

   ENDUIBLOCK(mWidth, mHeight);

   // Position arena to the right of the controls, like BouncingBalls does
   mArenaX = mWidth + 15;
   mArenaY = 10;
   mArenaW = 400;
   mArenaH = 300;

   // Expand mWidth/mHeight to encompass the arena
   mWidth += mArenaW + 30;
   mHeight = MAX(mHeight, mArenaH + 20);
}

SampleUniverse::~SampleUniverse()
{
   ClearSamples();
}

void SampleUniverse::Process(double time)
{
   PROFILER(SampleUniverse);

   if (!mEnabled)
      return;

   ComputeSliders(0);

   SyncOutputBuffer(2);
   mOutputBuffer.SetNumActiveChannels(2);
   int numChannels = mOutputBuffer.NumActiveChannels();
   int bufferSize = gBufferSize;

   mOutputBuffer.Clear();

   for (auto& voice : mVoices)
   {
      if (voice.playing && voice.sampleIndex >= 0 && voice.sampleIndex < (int)mSamples.size())
      {
         UniverseSample& us = mSamples[voice.sampleIndex];
         if (us.sample == nullptr) continue;
         ChannelBuffer* sampleData = us.sample->Data();
         int sampleLength = us.sample->LengthInSamples();
         int maxLen = MIN(sampleLength, (int)(gSampleRate * 0.3f)); // 300ms hit decay

         if (sampleLength > 0 && sampleData != nullptr)
         {
            float ratio = us.sample->GetSampleRateRatio();

            for (int i = 0; i < bufferSize; ++i)
            {
               if (voice.playPos < maxLen)
               {
                  int idx = (int)voice.playPos;
                  if (idx >= sampleLength) idx = sampleLength - 1;

                  float env = 1.0f;
                  float fadeOut = gSampleRate * 0.05f; // 50ms fade-out
                  if (voice.playPos > maxLen - fadeOut)
                     env = (maxLen - voice.playPos) / fadeOut;

                  for (int ch = 0; ch < numChannels; ++ch)
                  {
                     int dataChannel = MIN(ch, sampleData->NumActiveChannels() - 1);
                     float val = sampleData->GetChannel(dataChannel)[idx];
                     mOutputBuffer.GetChannel(ch)[i] += val * mVolume * env;
                  }

                  voice.playPos += ratio;
               }
               else
               {
                  voice.playing = false;
                  break;
               }
            }
         }
      }
   }

   // Path playback (transport-synced)
   if (mPlayingPath && !mRecordedPath.empty())
   {
      double measureTime = TheTransport->GetMeasureTime(time) - mPlayStartTime;
      double pathDuration = TheTransport->GetMeasureFraction(mQuantizeInterval);

      if (pathDuration <= 0.0001) pathDuration = 1.0;

      if (!mLoopPath && measureTime >= pathDuration)
      {
         mPlayingPath = false;
         mPlayPathCheckbox->SetValue(0, time);
      }
      else
      {
         bool wrapped = false;
         if (mLoopPath && measureTime >= pathDuration)
         {
            int completedLoops = (int)(measureTime / pathDuration);
            mPlayStartTime += completedLoops * pathDuration;
            measureTime = TheTransport->GetMeasureTime(time) - mPlayStartTime;
            mHoveredIndex = -1;
            wrapped = true;
         }

         double progress = measureTime / pathDuration;
         if (progress < 0.0) progress = 0.0;
         if (progress >= 1.0) progress = 0.9999;

         int numPts = (int)mRecordedPath.size();
         int rawPos = (int)(progress * numPts);
         if (rawPos >= numPts) rawPos = numPts - 1;
         if (rawPos < 0) rawPos = 0;

         int currPos = mReversePath ? (numPts - 1 - rawPos) : rawPos;
         int prevPos = ofClamp((int)mPlayheadPos, 0, numPts - 1);

         auto ScanSinglePos = [&](int scanPos) {
            if (scanPos >= 0 && scanPos < numPts)
            {
               ofVec2f pos = mRecordedPath[scanPos];
               for (int i = 0; i < (int)mSamples.size(); ++i)
               {
                  float dx = pos.x - mSamples[i].x;
                  float dy = pos.y - mSamples[i].y;
                  float distSq = dx * dx + dy * dy;
                  if (distSq < 64.0f && i != mHoveredIndex) // 8px radius
                  {
                     mHoveredIndex = i;
                     mSamples[i].hitTime = 1.0f;
                     TriggerSample(i, NextBufferTime(false));
                  }
               }
            }
         };

         auto ScanRange = [&](int start, int end, int step) {
            start = ofClamp(start, 0, numPts - 1);
            end = ofClamp(end, 0, numPts - 1);
            if (step > 0)
            {
               for (int p = start; p <= end; ++p)
                  ScanSinglePos(p);
            }
            else if (step < 0)
            {
               for (int p = start; p >= end; --p)
                  ScanSinglePos(p);
            }
         };

         if (wrapped)
         {
            if (!mReversePath)
            {
               ScanRange(prevPos + 1, numPts - 1, 1);
               ScanRange(0, currPos, 1);
            }
            else
            {
               ScanRange(prevPos - 1, 0, -1);
               ScanRange(numPts - 1, currPos, -1);
            }
         }
         else if (currPos != prevPos)
         {
            if (!mReversePath && currPos > prevPos)
            {
               ScanRange(prevPos + 1, currPos, 1);
            }
            else if (mReversePath && currPos < prevPos)
            {
               ScanRange(prevPos - 1, currPos, -1);
            }
         }

         mPlayheadPos = (float)currPos;
         mPlayheadProgress = (float)progress;
      }
   }

   // Output to target
   IAudioReceiver* targetOut = GetTarget(0);
   if (targetOut)
   {
      for (int ch = 0; ch < mOutputBuffer.NumActiveChannels(); ++ch)
      {
         Add(targetOut->GetBuffer()->GetChannel(ch), mOutputBuffer.GetChannel(ch), bufferSize);
      }
   }
   GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(0), bufferSize, 0);
   if (mOutputBuffer.NumActiveChannels() > 1)
      GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(1), bufferSize, 1);
}

void SampleUniverse::Poll()
{
}

void SampleUniverse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   // Draw all UI controls explicitly (like BouncingBalls)
   mSeedEntry->Draw();
   mPrevSeedButton->Draw();
   mReseedButton->Draw();
   mNextSeedButton->Draw();
   mClearButton->Draw();
   mRecordCheckbox->Draw();
   mPlayPathCheckbox->Draw();
   mLoopPathCheckbox->Draw();
   mReversePathCheckbox->Draw();
   mQuantizeLengthSelector->Draw();
   mShapeSelector->Draw();

   // Draw arena box
   ofPushStyle();
   ofSetColor(255, 255, 255, 255);
   ofNoFill();
   ofRect(mArenaX, mArenaY, mArenaW, mArenaH);

   // Draw recorded path
   if (!mRecordedPath.empty())
   {
      ofSetColor(100, 100, 100);
      ofBeginShape();
      for (const auto& p : mRecordedPath)
         ofVertex(mArenaX + p.x, mArenaY + p.y);
      ofEndShape();

      // Draw smooth playhead using continuous progress
      if (mPlayingPath)
      {
         float t = mPlayheadProgress;
         if (mReversePath)
            t = 1.0f - t;

         // Interpolate smoothly along path
         float fIdx = t * (mRecordedPath.size() - 1);
         int idx0 = (int)fIdx;
         int idx1 = idx0 + 1;
         if (idx0 < 0) idx0 = 0;
         if (idx1 >= (int)mRecordedPath.size()) idx1 = (int)mRecordedPath.size() - 1;
         float frac = fIdx - idx0;

         float px = mRecordedPath[idx0].x + (mRecordedPath[idx1].x - mRecordedPath[idx0].x) * frac;
         float py = mRecordedPath[idx0].y + (mRecordedPath[idx1].y - mRecordedPath[idx0].y) * frac;

         ofSetColor(255, 255, 255);
         ofFill();
         ofCircle(mArenaX + px, mArenaY + py, 5);
      }
   }

   // Draw sample dots with hit animation
   ofFill();
   float dt = ofGetLastFrameTime();
   for (int i = 0; i < (int)mSamples.size(); ++i)
   {
      // Decay hit animation
      if (mSamples[i].hitTime > 0)
         mSamples[i].hitTime -= dt * 6.0f; // decay over ~170ms
      if (mSamples[i].hitTime < 0)
         mSamples[i].hitTime = 0;

      float bump = mSamples[i].hitTime;
      float baseR = 6.0f;
      float r = baseR + bump * 8.0f; // pop from 6 to 14 on hit

      // Flash brighter on hit
      int brightBoost = (int)(bump * 100);
      ofColor c = mSamples[i].color;
      c.r = MIN(255, c.r + brightBoost);
      c.g = MIN(255, c.g + brightBoost);
      c.b = MIN(255, c.b + brightBoost);
      ofSetColor(c);

      ofCircle(mArenaX + mSamples[i].x, mArenaY + mSamples[i].y, r);
   }

   ofPopStyle();
}

void SampleUniverse::Resize(float w, float h)
{
}

void SampleUniverse::FilesDropped(std::vector<std::string> files, int x, int y)
{
   for (auto& file : files)
   {
      if (mSamples.size() >= 100)
         break;

      Sample* s = new Sample();
      s->Read(file.c_str());

      if (s->LengthInSamples() == 0)
      {
         delete s;
         continue;
      }

      UniverseSample us;
      us.sample = s;
      us.color = ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255));
      mSamples.push_back(us);
   }
   RespawnDots();
}

void SampleUniverse::SampleDropped(int x, int y, Sample* sample)
{
   if (mSamples.size() >= 100)
      return;

   if (sample == nullptr || sample->LengthInSamples() == 0)
      return;

   UniverseSample us;
   us.sample = new Sample();
   us.sample->CopyFrom(sample);
   us.color = ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255));
   mSamples.push_back(us);

   RespawnDots();
}

void SampleUniverse::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   float rx = x - mArenaX;
   float ry = y - mArenaY;

   if (rx >= 0 && rx <= mArenaW && ry >= 0 && ry <= mArenaH)
   {
      mIsDrawing = true;
      if (mRecording)
      {
         mRecordedPath.clear();
         mRecordedPath.push_back(ofVec2f(rx, ry));
      }

      int closest = -1;
      float closestDist = 15.0f;
      for (int i = 0; i < (int)mSamples.size(); ++i)
      {
         float dx = rx - mSamples[i].x;
         float dy = ry - mSamples[i].y;
         float d = sqrt(dx * dx + dy * dy);
         if (d < closestDist)
         {
            closestDist = d;
            closest = i;
         }
      }
      if (closest != -1)
      {
         mHoveredIndex = closest;
         TriggerSample(closest, NextBufferTime(false));
      }
   }
}

bool SampleUniverse::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (!mIsDrawing) return false;

   float rx = x - mArenaX;
   float ry = y - mArenaY;

   if (rx >= 0 && rx <= mArenaW && ry >= 0 && ry <= mArenaH)
   {
      if (mRecording)
      {
         mRecordedPath.push_back(ofVec2f(rx, ry));
      }

      int closest = -1;
      float closestDist = 15.0f;
      for (int i = 0; i < (int)mSamples.size(); ++i)
      {
         float dx = rx - mSamples[i].x;
         float dy = ry - mSamples[i].y;
         float d = sqrt(dx * dx + dy * dy);
         if (d < closestDist)
         {
            closestDist = d;
            closest = i;
         }
      }
      if (closest != -1 && mHoveredIndex != closest)
      {
         mHoveredIndex = closest;
         TriggerSample(closest, NextBufferTime(false));
      }
      else if (closest == -1)
      {
         mHoveredIndex = -1;
      }
   }
   else
   {
      mHoveredIndex = -1;
   }
   return true;
}

void SampleUniverse::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mIsDrawing = false;
}

static ofVec2f GetPolygonPoint(const std::vector<ofVec2f>& points, float t)
{
   if (points.empty()) return ofVec2f(0.5f, 0.5f);
   if (points.size() == 1) return points[0];
   
   float p = t * (points.size() - 1);
   int idx = (int)p;
   if (idx >= points.size() - 1) return points.back();
   float frac = p - idx;
   ofVec2f v1 = points[idx];
   ofVec2f v2 = points[idx + 1];
   return ofVec2f(v1.x + (v2.x - v1.x) * frac, v1.y + (v2.y - v1.y) * frac);
}

ofVec2f SampleUniverse::GetShapePosition(ShapePreset shape, float t, int i, int N)
{
   float nx = 0.5f, ny = 0.5f;
   int cols = ceil(sqrt(N));
   int rows = ceil((float)N / cols);

   switch (shape)
   {
      case kShape_Random:
         nx = DeterministicRandomFloat01(mSeed, i * 2);
         ny = DeterministicRandomFloat01(mSeed, i * 2 + 1);
         break;
      case kShape_Line:
         nx = t;
         break;
      case kShape_Circle:
      {
         float theta = t * 2 * PI;
         nx = 0.5f + 0.45f * cos(theta);
         ny = 0.5f + 0.45f * sin(theta);
         break;
      }
      case kShape_Spiral:
      {
         float theta = t * 6 * PI;
         float r = 0.45f * t;
         nx = 0.5f + r * cos(theta);
         ny = 0.5f + r * sin(theta);
         break;
      }
      case kShape_Square:
      {
         float p = t * 4.0f;
         if (p < 1.0f) { nx = p; ny = 0.0f; }
         else if (p < 2.0f) { nx = 1.0f; ny = p - 1.0f; }
         else if (p < 3.0f) { nx = 1.0f - (p - 2.0f); ny = 1.0f; }
         else { nx = 0.0f; ny = 1.0f - (p - 3.0f); }
         break;
      }
      case kShape_Triangle:
      {
         float p = t * 3.0f;
         if (p < 1.0f) { nx = p; ny = 1.0f; }
         else if (p < 2.0f) { nx = 1.0f - (p - 1.0f) * 0.5f; ny = 1.0f - (p - 1.0f); }
         else { nx = 0.5f - (p - 2.0f) * 0.5f; ny = p - 2.0f; }
         break;
      }
      case kShape_Cross:
      {
         if (t < 0.5f) { nx = t * 2.0f; ny = 0.5f; }
         else { nx = 0.5f; ny = (t - 0.5f) * 2.0f; }
         break;
      }
      case kShape_Star:
      {
         int seg = (int)(t * 5.0f);
         if (seg > 4) seg = 4;
         float p = (t * 5.0f) - seg;
         float t1 = (seg * 2 * 2 * PI / 5.0f) - PI / 2.0f;
         float t2 = ((seg + 1) * 2 * 2 * PI / 5.0f) - PI / 2.0f;
         float v1x = 0.5f + 0.45f * cos(t1);
         float v1y = 0.5f + 0.45f * sin(t1);
         float v2x = 0.5f + 0.45f * cos(t2);
         float v2y = 0.5f + 0.45f * sin(t2);
         nx = v1x + (v2x - v1x) * p;
         ny = v1y + (v2y - v1y) * p;
         break;
      }
      case kShape_Torus:
      {
         float u = t * 2 * PI;
         float v = t * 20 * PI;
         nx = 0.5f + (0.3f + 0.15f * cos(v)) * cos(u);
         ny = 0.5f + (0.3f + 0.15f * cos(v)) * sin(u);
         break;
      }
      case kShape_Cat:
      {
         static const std::vector<ofVec2f> pts = { {0.1f, 0.1f}, {0.3f, 0.3f}, {0.7f, 0.3f}, {0.9f, 0.1f}, {0.9f, 0.7f}, {0.5f, 0.9f}, {0.1f, 0.7f}, {0.1f, 0.1f} };
         ofVec2f pt = GetPolygonPoint(pts, t);
         nx = pt.x; ny = pt.y;
         break;
      }
      case kShape_Frog:
      {
         static const std::vector<ofVec2f> pts = { {0.2f, 0.1f}, {0.4f, 0.3f}, {0.6f, 0.3f}, {0.8f, 0.1f}, {0.9f, 0.5f}, {0.5f, 0.9f}, {0.1f, 0.5f}, {0.2f, 0.1f} };
         ofVec2f pt = GetPolygonPoint(pts, t);
         nx = pt.x; ny = pt.y;
         break;
      }
      case kShape_Computer:
      {
         static const std::vector<ofVec2f> pts = { {0.1f, 0.1f}, {0.9f, 0.1f}, {0.9f, 0.6f}, {0.1f, 0.6f}, {0.1f, 0.1f}, {0.4f, 0.6f}, {0.4f, 0.9f}, {0.2f, 0.9f}, {0.8f, 0.9f}, {0.6f, 0.9f}, {0.6f, 0.6f} };
         ofVec2f pt = GetPolygonPoint(pts, t);
         nx = pt.x; ny = pt.y;
         break;
      }
   }
   return ofVec2f(nx, ny);
}

void SampleUniverse::RespawnDots()
{
   if (mSamples.empty()) return;

   int N = (int)mSamples.size();
   for (int i = 0; i < N; ++i)
   {
      float t = N > 1 ? (float)i / (N - 1) : 0.5f;
      ofVec2f npos = GetShapePosition(mCurrentShape, t, i, N);
      mSamples[i].x = 10.0f + npos.x * (mArenaW - 20.0f);
      mSamples[i].y = 10.0f + npos.y * (mArenaH - 20.0f);
   }
   
   // Generate a deterministic random path through all dots
   GenerateRandomPath();
}

void SampleUniverse::Reseed()
{
   mSeed = gRandom() % 10000;
   RespawnDots();
}

void SampleUniverse::GenerateRandomPath()
{
   mRecordedPath.clear();
   
   if (mSamples.empty())
      return;
   
   if (mCurrentShape == kShape_Random)
   {
      std::vector<int> order;
      for (int i = 0; i < (int)mSamples.size(); ++i)
         order.push_back(i);
      
      for (int i = (int)order.size() - 1; i > 0; --i)
      {
         int j = (int)(DeterministicRandomFloat01(mSeed, 1000 + i) * (i + 1));
         if (j > i) j = i;
         std::swap(order[i], order[j]);
      }
      
      int numPointsPerSegment = 20;
      for (int seg = 0; seg < (int)order.size(); ++seg)
      {
         int from = order[seg];
         int to = order[(seg + 1) % order.size()];
         
         for (int p = 0; p < numPointsPerSegment; ++p)
         {
            float t = (float)p / numPointsPerSegment;
            float x = mSamples[from].x + (mSamples[to].x - mSamples[from].x) * t;
            float y = mSamples[from].y + (mSamples[to].y - mSamples[from].y) * t;
            mRecordedPath.push_back(ofVec2f(x, y));
         }
      }
   }
   else
   {
      int numPoints = 1000;
      for (int p = 0; p < numPoints; ++p)
      {
         float t = (float)p / (numPoints - 1);
         ofVec2f npos = GetShapePosition(mCurrentShape, t, p, numPoints);
         float x = 10.0f + npos.x * (mArenaW - 20.0f);
         float y = 10.0f + npos.y * (mArenaH - 20.0f);
         mRecordedPath.push_back(ofVec2f(x, y));
      }
   }
}

void SampleUniverse::ClearSamples()
{
   for (auto& us : mSamples)
   {
      if (us.sample) delete us.sample;
   }
   mSamples.clear();
   mHoveredIndex = -1;

   for (auto& voice : mVoices)
   {
      voice.playing = false;
      voice.sampleIndex = -1;
      voice.playPos = 0.0;
   }

   mRecordedPath.clear();
   mPlayheadPos = 0.0f;
   mPlayheadProgress = 0.0f;
}

void SampleUniverse::TriggerSample(int index, double time)
{
   if (index < 0 || index >= (int)mSamples.size())
      return;

   // Find free voice, or steal the oldest (most-advanced) one
   int voiceIdx = -1;
   float oldestPos = -1.0f;
   int oldestIdx = 0;
   for (int i = 0; i < (int)mVoices.size(); ++i)
   {
      if (!mVoices[i].playing)
      {
         voiceIdx = i;
         break;
      }
      if (mVoices[i].playPos > oldestPos)
      {
         oldestPos = mVoices[i].playPos;
         oldestIdx = i;
      }
   }

   if (voiceIdx == -1)
      voiceIdx = oldestIdx; // steal oldest voice

   mVoices[voiceIdx].sampleIndex = index;
   mVoices[voiceIdx].playPos = 0;
   mVoices[voiceIdx].playing = true;
}

void SampleUniverse::TextEntryComplete(TextEntry* entry)
{
   if (entry == mSeedEntry)
   {
      RespawnDots();
   }
}

void SampleUniverse::ButtonClicked(ClickButton* button, double time)
{
   if (button == mReseedButton)
   {
      Reseed();
      mSeedEntry->SetText(std::to_string(mSeed));
   }
   else if (button == mPrevSeedButton)
   {
      if (mSeed > 0)
      {
         --mSeed;
         RespawnDots();
         mSeedEntry->SetText(std::to_string(mSeed));
      }
   }
   else if (button == mNextSeedButton)
   {
      if (mSeed < 9999)
      {
         ++mSeed;
         RespawnDots();
         mSeedEntry->SetText(std::to_string(mSeed));
      }
   }
   else if (button == mClearButton)
   {
      ClearSamples();
   }
}

void SampleUniverse::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mPlayPathCheckbox)
   {
      if (mPlayingPath)
      {
         mPlayStartTime = TheTransport->GetMeasureTime(time);
         int numPts = (int)mRecordedPath.size();
         int startPos = (mReversePath && numPts > 0) ? (numPts - 1) : 0;
         mPlayheadPos = (float)startPos;
         mPlayheadProgress = 0.0f;
         mHoveredIndex = -1;
      }
   }
}

void SampleUniverse::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mShapeSelector)
   {
      RespawnDots();
   }
}

void SampleUniverse::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void SampleUniverse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SampleUniverse::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   RespawnDots();
}

void SampleUniverse::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   out << mSeed;
   
   out << (int)mCurrentShape;

   // save samples
   out << (int)mSamples.size();
   for (auto& us : mSamples)
      us.sample->SaveState(out);
}

void SampleUniverse::LoadState(FileStreamIn& in, int rev)
{
   in >> mSeed;
   mSeedEntry->SetText(std::to_string(mSeed));
   
   if (rev >= 2)
   {
      int shape;
      in >> shape;
      mCurrentShape = (ShapePreset)shape;
      mShapeSelector->SetValue(mCurrentShape, 0);
   }

   int numSamples;
   in >> numSamples;
   for (int i = 0; i < numSamples; ++i)
   {
      UniverseSample us;
      us.sample = new Sample();
      us.sample->LoadState(in);
      us.color = ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255));
      mSamples.push_back(us);
   }

   RespawnDots();
}

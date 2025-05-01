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
//  BeatBloks.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/9/13.
//
//

#include "BeatBloks.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

const float mBufferX = 5;
const float mBufferY = 80;
const float mBufferW = 900;
const float mBufferH = 200;
const float mRemixBufferY = mBufferY + mBufferH + 60;

BeatBloks::BeatBloks()
: mRemixZoomEnd(gSampleRate * 25)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   mSample = new Sample();

   /*vector<string> fake;
   fake.push_back(ofToDataPath("01 - Reflektor.mp3",true));
   FilesDropped(fake, 0, 0);*/
}

void BeatBloks::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "volume", 5, 20, 110, 15, &mVolume, 0, 2);
   mPlayCheckbox = new Checkbox(this, "play", 5, mBufferY - 20, &mPlay);
   mLoopCheckbox = new Checkbox(this, "loop", 60, mBufferY - 20, &mLoop);
   mClipStartSlider = new FloatSlider(this, "start", mBufferX, mBufferY + mBufferH - 30, 900, 15, &mClipStart, 0, gSampleRate * 200);
   mClipEndSlider = new FloatSlider(this, "end", mBufferX, mBufferY + mBufferH - 15, 900, 15, &mClipEnd, 0, gSampleRate * 200);
   mZoomStartSlider = new FloatSlider(this, "zoomstart", mBufferX, mBufferY + mBufferH + 5, 900, 15, &mZoomStart, 0, gSampleRate * 200);
   mZoomEndSlider = new FloatSlider(this, "zoomend", mBufferX, mBufferY + mBufferH + 20, 900, 15, &mZoomEnd, 0, gSampleRate * 200);
   mNumBarsSlider = new IntSlider(this, "num bars", 215, 3, 220, 15, &mNumBars, 1, 16);
   mOffsetSlider = new FloatSlider(this, "offset", 215, 20, 300, 15, &mOffset, -gSampleRate * 2, gSampleRate * 2);
   mWriteButton = new ClickButton(this, "[test]", 600, 50);
   mDoubleLengthButton = new ClickButton(this, "double", 600, 10);
   mHalveLengthButton = new ClickButton(this, "halve", 600, 28);
   mDrawBlokTypeDropdown = new DropdownList(this, "draw type", 215, 40, (int*)&mDrawBlokType);
   mPlayRemixCheckbox = new Checkbox(this, "play remix", 15, mRemixBufferY - 20, &mPlayRemix);
   mClearRemixButton = new ClickButton(this, "clear remix", 100, mRemixBufferY - 20);
   mDrawSourcesCheckbox = new Checkbox(this, "draw sources", 300, mRemixBufferY - 20, &mDrawSources);
   mRemixZoomStartSlider = new FloatSlider(this, "r zoomstart", mBufferX, mRemixBufferY + mBufferH + 5, 900, 15, &mRemixZoomStart, 0, gSampleRate * 200);
   mRemixZoomEndSlider = new FloatSlider(this, "r zoomend", mBufferX, mRemixBufferY + mBufferH + 20, 900, 15, &mRemixZoomEnd, 0, gSampleRate * 200);
   mGetLuckyButton = new ClickButton(this, "gl", 700, 10);
   mLoseYourselfButton = new ClickButton(this, "ly", 700, 28);

   mDrawBlokTypeDropdown->AddLabel("tatums", kBlok_Tatum);
   mDrawBlokTypeDropdown->AddLabel("beats", kBlok_Beat);
   mDrawBlokTypeDropdown->AddLabel("segments", kBlok_Segment);
   mDrawBlokTypeDropdown->AddLabel("bars", kBlok_Bar);
   mDrawBlokTypeDropdown->AddLabel("sections", kBlok_Section);
}

BeatBloks::~BeatBloks()
{
   delete[] mWriteBuffer;
   delete mSample;
}

void BeatBloks::Poll()
{
   if (mBlockMultiPlaceEngaged && !IsKeyHeld('z'))
   {
      mHeldBlok = nullptr;
   }
}

void BeatBloks::Process(double time)
{
   PROFILER(BeatBloks);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr || mSample == nullptr || mLoading)
      return;

   ComputeSliders(0);

   if (mWantWrite)
   {
      DoWrite();
      mWantWrite = false;
   }

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume;

   float clipStart = mClipStart;
   float clipEnd = mClipEnd;
   if (mPlay && (clipStart < clipEnd))
   {
      float speed = float(clipEnd - clipStart) * gInvSampleRateMs / TheTransport->MsPerBar() / mNumBars;

      const float* data = mSample->Data()->GetChannel(0);
      int numSamples = mSample->LengthInSamples();
      float sampleRateRatio = mSample->GetSampleRateRatio();

      mPlayheadRemainder = TheTransport->GetMeasurePos(time) + (TheTransport->GetMeasure(time) % mNumBars);
      mPlayheadRemainder /= mNumBars;
      mPlayheadRemainder *= clipEnd - clipStart;
      mPlayheadWhole = int(mPlayheadRemainder);
      mPlayheadRemainder -= mPlayheadWhole;
      mPlayheadWhole += int(clipStart);
      mPlayheadRemainder += clipStart - int(clipStart);
      mPlayheadWhole = MAX(0, mPlayheadWhole);
      mPlayheadRemainder = MAX(0.0f, mPlayheadRemainder);

      for (int i = 0; i < bufferSize; ++i)
      {
         if (mPlayheadWhole >= clipEnd)
            mPlayheadWhole -= (clipEnd - clipStart);
         if (mPlayheadWhole < clipStart)
            mPlayheadWhole += (clipEnd - clipStart);

         out[i] = GetInterpolatedSample(mPlayheadRemainder + 1, data + mPlayheadWhole - 1, numSamples) * volSq;

         mPlayheadRemainder += speed * sampleRateRatio;
      }
   }

   Blok* heldBlok = mHeldBlok; //hold onto it in case it gets nulled in the main thread
   if (mPlayBlokPreview && heldBlok)
   {
      float speed = 1;

      const float* data = mSample->Data()->GetChannel(0);
      int numSamples = mSample->LengthInSamples();
      float sampleRateRatio = mSample->GetSampleRateRatio();

      double previewTime = time;
      for (int i = 0; i < bufferSize; ++i)
      {
         if (mBlokPreviewPlayhead == 0)
         {
            mBlokPreviewRamp.Start(previewTime, 1, previewTime + 1);
         }
         if (mBlokPreviewPlayhead > heldBlok->mDuration * numSamples)
         {
            if (mBlokPreviewRamp.Target(previewTime) != 0)
               mBlokPreviewRamp.Start(previewTime, 0, previewTime + 1);

            if (mBlokPreviewRamp.Value(previewTime) == 0)
            {
               mPlayBlokPreview = false;
            }
         }

         float lookupPlayhead = StartTime(*heldBlok) * numSamples + mBlokPreviewPlayhead;

         out[i] = GetInterpolatedSample(lookupPlayhead, data, numSamples);
         out[i] *= mBlokPreviewRamp.Value(previewTime);
         out[i] *= volSq;

         mBlokPreviewPlayhead += speed * sampleRateRatio;
         previewTime += gInvSampleRateMs;
      }
   }

   if (mPlayRemix)
   {
      float speed = 1;

      const float* data = mSample->Data()->GetChannel(0);
      int numSamples = mSample->LengthInSamples();
      float sampleRateRatio = mSample->GetSampleRateRatio();

      for (int i = 0; i < bufferSize; ++i)
      {
         float remixPlayheadLeft = mRemixPlayhead;
         Blok* currentBlok = nullptr;
         for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter)
         {
            if (remixPlayheadLeft < (*iter)->mDuration * numSamples)
            {
               currentBlok = (*iter);
               break;
            }

            remixPlayheadLeft -= (*iter)->mDuration * numSamples;
         }

         if (currentBlok == nullptr)
         {
            out[i] = 0;
            mPlayRemix = false;
            continue;
         }

         if (currentBlok != mLastPlayedRemixBlok && mLastPlayedRemixBlok)
         {
            mRemixJumpBlender.CaptureForJump(mLastLookupPlayhead, data, numSamples, i);
         }

         float lookupPlayhead = StartTime(*currentBlok) * numSamples + remixPlayheadLeft;
         mLastLookupPlayhead = lookupPlayhead;

         out[i] = GetInterpolatedSample(lookupPlayhead, data, numSamples);
         out[i] = mRemixJumpBlender.Process(out[i], i);
         out[i] *= volSq;

         mRemixPlayhead += speed * sampleRateRatio;

         mPlayheadWhole = int(lookupPlayhead); //for visualizing it jump around

         mLastPlayedRemixBlok = currentBlok;
      }
   }

   GetVizBuffer()->WriteChunk(out, bufferSize, 0);
}

void BeatBloks::FilesDropped(std::vector<std::string> files, int x, int y)
{
   mLoading = true;

   mSample->Reset();

   mSample->Read(files[0].c_str());

   ResetRead();

   std::vector<std::string> tokens = ofSplitString(files[0].c_str(), GetPathSeparator());
   std::string cachedFilename = tokens[tokens.size() - 1].c_str();
   tokens = ofSplitString(cachedFilename, ".");
   cachedFilename = tokens[0] + ".cached";
   bool hasCached = juce::File(ofToDataPath(cachedFilename)).existsAsFile();

   ofLog() << cachedFilename << " exists: " << (hasCached ? "true" : "false");

   FILE* output;
   FILE* cachedFile;
   if (!hasCached) //have to look it up with echonest
   {
      char command[2048];
      snprintf(command, sizeof(command), "export ECHO_NEST_API_KEY=SUZ3W7PAIVQQXZCAW; export PATH=/usr/local/bin:$PATH; python \"%s\" \"%s\"", ofToDataPath("get_echonest_remix_data.py").c_str(), files[0].c_str());
      output = popen(command, "r");
      cachedFile = fopen(ofToDataPath(cachedFilename).c_str(), "w");
   }
   else
   {
      output = fopen(ofToDataPath(cachedFilename).c_str(), "r");
   }

   char c;
   char line[512]{};
   int linepos = 0;
   do
   {
      c = fgetc(output);
      if (!hasCached)
         fputc(c, cachedFile);
      //printf("%c",c);
      if (c == '\n' || c == EOF)
      {
         ReadEchonestLine(line);

         linepos = 0;
         std::memset(line, 0, sizeof(line));
      }
      else
      {
         line[linepos++] = c;
      }
   } while (c != EOF);

   if (hasCached)
   {
      fclose(output);
   }
   else
   {
      pclose(output);
      fclose(cachedFile);
   }

   mClipStart = 0;
   mClipEnd = mSample->LengthInSamples();
   mZoomStart = 0;
   mZoomEnd = MIN(1400000, mClipEnd);
   mNumBars = (int)mBars.size();
   mZoomStartSlider->SetExtents(0, mSample->LengthInSamples());
   mZoomEndSlider->SetExtents(0, mSample->LengthInSamples());
   UpdateZoomExtents();

   mRemixBloks.clear();
   mOffset = 0;

   TheTransport->SetTempo(76);

   mLoading = false;
}

void BeatBloks::ResetRead()
{
   mReadState = kReadState_Start;
   mBars.clear();
   mBeats.clear();
   mTatums.clear();
   mSections.clear();
   mSegments.clear();
}

void BeatBloks::ReadEchonestLine(const char* line)
{
   std::vector<std::string> tokens = ofSplitString(line, " ");
   if (tokens.size() == 1)
   {
      if (tokens[0] == "bars")
         mReadState = kReadState_Bars;
      if (tokens[0] == "beats")
         mReadState = kReadState_Beats;
      if (tokens[0] == "tatums")
         mReadState = kReadState_Tatums;
      if (tokens[0] == "sections")
         mReadState = kReadState_Sections;
      if (tokens[0] == "segments")
         mReadState = kReadState_Segments;
   }
   else
   {
      float lengthInSeconds = mSample->LengthInSamples() / gSampleRate;

      if (mReadState == kReadState_Bars || mReadState == kReadState_Beats || mReadState == kReadState_Tatums)
         assert(tokens.size() == 3);
      if (mReadState == kReadState_Sections || mReadState == kReadState_Segments)
         assert(tokens.size() == 2);

      float adjustSeconds = -.062f;
      float start = (ofToFloat(tokens[0]) + adjustSeconds) / lengthInSeconds;
      float duration = ofToFloat(tokens[1]) / lengthInSeconds;
      float confidence = tokens.size() == 3 ? ofToFloat(tokens[2]) : 1;

      Blok blok(start, duration, confidence);
      if (mReadState == kReadState_Bars)
      {
         blok.mType = kBlok_Bar;
         mBars.push_back(blok);
      }
      if (mReadState == kReadState_Beats)
      {
         blok.mType = kBlok_Beat;
         mBeats.push_back(blok);
      }
      if (mReadState == kReadState_Tatums)
      {
         blok.mType = kBlok_Tatum;
         mTatums.push_back(blok);
      }
      if (mReadState == kReadState_Sections)
      {
         blok.mType = kBlok_Section;
         mSections.push_back(blok);
      }
      if (mReadState == kReadState_Segments)
      {
         blok.mType = kBlok_Segment;
         mSegments.push_back(blok);
      }
   }
}

void BeatBloks::DropdownClicked(DropdownList* list)
{
}

void BeatBloks::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void BeatBloks::UpdateSample()
{
}

void BeatBloks::ButtonClicked(ClickButton* button, double time)
{
   if (button == mWriteButton)
   {
      //mWantWrite = true;
      mClipStart = StartTime(mBars[10]) * mSample->LengthInSamples();
      mClipEnd = StartTime(mBars[14]) * mSample->LengthInSamples();
      mZoomStart = mClipStart - 40000;
      mZoomEnd = mClipEnd + 40000;
      mNumBars = 4;
      UpdateZoomExtents();
   }
   if (button == mDoubleLengthButton)
   {
      float newEnd = (mClipEnd - mClipStart) * 2 + mClipStart;
      if (newEnd < mSample->LengthInSamples())
      {
         mClipEnd = newEnd;
         mNumBars *= 2;
      }
   }
   if (button == mHalveLengthButton)
   {
      if (mNumBars % 2 == 0)
      {
         float newEnd = (mClipEnd - mClipStart) / 2 + mClipStart;
         mClipEnd = newEnd;
         mNumBars /= 2;
      }
   }
   if (button == mClearRemixButton)
   {
      mRemixBloks.clear();
   }

   if (button == mGetLuckyButton)
   {
      std::vector<std::string> fake;
      fake.push_back(ofToDataPath("Daft Punk - Get Lucky.mp3"));
      FilesDropped(fake, 0, 0);
      mOffset = -53275;
   }
   if (button == mLoseYourselfButton)
   {
      std::vector<std::string> fake;
      fake.push_back(ofToDataPath("Daft Punk - Lose Yourself To Dance.mp3"));
      FilesDropped(fake, 0, 0);
   }
}

void BeatBloks::DoWrite()
{
   if (mSample)
   {
      mSample->ClipTo(mClipStart, mClipEnd);
      int shift = (mClipEnd - mClipStart);
      if (shift < 0)
         shift += mClipEnd - mClipStart;
      mSample->ShiftWrap(shift);
      mSample->Write(ofGetTimestampString("BeatBloks_%Y-%m-%d_%H-%M.wav").c_str());
      mClipStart = 0;
      mClipEnd = mSample->LengthInSamples();

      mZoomStart = 0;
      mZoomEnd = mClipEnd;
      mZoomStartSlider->SetExtents(0, mClipEnd);
      mZoomEndSlider->SetExtents(0, mClipEnd);
      UpdateZoomExtents();
   }
}

void BeatBloks::UpdateZoomExtents()
{
   mClipStartSlider->SetExtents(mZoomStart, mZoomEnd);
   mClipEndSlider->SetExtents(mZoomStart, mZoomEnd);
}

void BeatBloks::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   int sampleLength = mSample->LengthInSamples();


   if (x >= mBufferX && y >= mBufferY && x <= mBufferX + mBufferW && y <= mBufferY + mBufferH - 28) //clicked waveform
   {
      x -= mBufferX;
      y -= mBufferY;
      if (y < MEASURE_ZONE_HEIGHT) //clicked measure
      {
         float zoomPos = x / mBufferW;
         float pos = ofMap(zoomPos, 0, 1, mZoomStart / sampleLength, mZoomEnd / sampleLength);
         for (int i = 0; i < mBars.size(); ++i)
         {
            if (StartTime(mBars[i]) < pos && pos < StartTime(mBars[i]) + mBars[i].mDuration)
               mHeldBlok = &mBars[i];
         }
      }
      else if (y > mBufferH - BEAT_ZONE_HEIGHT) //clicked beat
      {
         float zoomPos = x / mBufferW;
         float pos = ofMap(zoomPos, 0, 1, mZoomStart / sampleLength, mZoomEnd / sampleLength);
         for (int i = 0; i < mBeats.size(); ++i)
         {
            if (StartTime(mBeats[i]) < pos && pos < StartTime(mBeats[i]) + mBeats[i].mDuration)
               mHeldBlok = &mBeats[i];
         }
      }
      else //clicked tatum
      {
         float zoomPos = x / mBufferW;
         float pos = ofMap(zoomPos, 0, 1, mZoomStart / sampleLength, mZoomEnd / sampleLength);
         for (int i = 0; i < mTatums.size(); ++i)
         {
            if (StartTime(mTatums[i]) < pos && pos < StartTime(mTatums[i]) + mTatums[i].mDuration)
               mHeldBlok = &mTatums[i];
         }
      }

      if (mHeldBlok)
      {
         if (IsKeyHeld('s'))
         {
            mRemixBloks.push_back(mHeldBlok);
            mHeldBlok = nullptr;
         }
         else
         {
            float pos = ofMap(StartTime(*mHeldBlok), mZoomStart / sampleLength, mZoomEnd / sampleLength, 0, 1);
            mGrabOffsetX = x - mBufferW * pos;

            mGrabOffsetY = y;

            mBlokPreviewPlayhead = 0;
            mPlayBlokPreview = true;
            mBlockMultiPlaceEngaged = false;
         }

         mLastRemovedRemixBlokIdx = -1;
      }
   }
   else if (y > mRemixBufferY && y < mRemixBufferY + mBufferH)
   {
      if (mHeldBlok)
      {
         PlaceHeldBlok();
         if (!IsKeyHeld('z'))
            mHeldBlok = nullptr;
      }
      else if (IsKeyHeld('a'))
      {
         mHeldBlok = RemoveBlokAt(x);
         mBlokPreviewPlayhead = 0;
         mPlayBlokPreview = true;
         mBlockMultiPlaceEngaged = false;
         mGrabOffsetX = 0;
         mGrabOffsetY = 0;
      }
      else if (IsKeyHeld('x'))
      {
         RemoveBlokAt(x);
      }
      else
      {
         mRemixPlayhead = ofMap(x, mBufferX, mBufferX + mBufferW, mRemixZoomStart, mRemixZoomEnd, true);
         mPlayRemix = true;
      }
   }
   else
   {
      mHeldBlok = nullptr;
   }
}

void BeatBloks::PlaceHeldBlok()
{
   assert(mHeldBlok);

   int insertIndex;
   GetInsertPosition(insertIndex);
   if (insertIndex == -1)
   {
      mRemixBloks.push_back(mHeldBlok);
   }
   else
   {
      int i = 0;
      for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter, ++i)
      {
         if (i == insertIndex)
         {
            mRemixBloks.insert(iter, mHeldBlok);
            break;
         }
      }
   }
}

void BeatBloks::MouseReleased()
{
   IDrawableModule::MouseReleased();

   if (!mBlockMultiPlaceEngaged && mMouseY > mRemixBufferY && mMouseY < mRemixBufferY + mBufferH)
   {
      if (mHeldBlok)
         PlaceHeldBlok();
   }

   if (IsKeyHeld('z'))
      mBlockMultiPlaceEngaged = true;
   else
      mHeldBlok = nullptr;
}

bool BeatBloks::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mMouseX = x;
   mMouseY = y;

   return false;
}

void BeatBloks::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
   mPlayCheckbox->Draw();
   mLoopCheckbox->Draw();
   mDrawBlokTypeDropdown->Draw();
   mPlayRemixCheckbox->Draw();
   mClearRemixButton->Draw();
   mDrawSourcesCheckbox->Draw();
   mRemixZoomStartSlider->Draw();
   mRemixZoomEndSlider->Draw();
   mGetLuckyButton->Draw();
   mLoseYourselfButton->Draw();

   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(mBufferX, mBufferY);
      ofPushStyle();

      mSample->LockDataMutex(true);
      DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), mZoomStart, mZoomEnd, (int)mPlayheadWhole);
      mSample->LockDataMutex(false);

      int sampleLength = MAX(1, mSample->LengthInSamples());

      if (mBars.size() != 0)
      {
         ofSetColor(0, 255, 255, 60);
         std::vector<Blok>& drawBloks = mNothing;
         switch (mDrawBlokType)
         {
            case kBlok_Bar:
               drawBloks = mBars;
               break;
            case kBlok_Beat:
               drawBloks = mBeats;
               break;
            case kBlok_Tatum:
               drawBloks = mTatums;
               break;
            case kBlok_Section:
               drawBloks = mSections;
               break;
            case kBlok_Segment:
               drawBloks = mSegments;
               break;
         }

         for (int i = 0; i < drawBloks.size(); ++i)
         {
            float pos = ofMap(StartTime(drawBloks[i]), mZoomStart / sampleLength, mZoomEnd / sampleLength, 0, 1);
            if (pos >= 0 && pos <= 1)
               ofLine(mBufferW * pos, 0, mBufferW * pos, mBufferH);
         }

         ofFill();
         for (int i = 0; i < mBars.size(); ++i)
         {
            ofSetColor(255, 0, (i % 2) * 255, 60);
            float pos = ofMap(StartTime(mBars[i]), mZoomStart / sampleLength, mZoomEnd / sampleLength, 0, 1);
            float dur = ofMap(mBars[i].mDuration, 0, (mZoomEnd - mZoomStart) / sampleLength, 0, 1);
            float end = pos + dur;
            if ((pos >= 0 && pos <= 1) || (end >= 0 && end <= 1))
            {
               pos = ofClamp(pos, 0, 1);
               end = ofClamp(end, 0, 1);
               ofRect(mBufferW * pos, 0, mBufferW * (end - pos), MEASURE_ZONE_HEIGHT);
            }
         }

         for (int i = 0; i < mBeats.size(); ++i)
         {
            ofSetColor(255, 255, (i % 2) * 255, 60);
            float pos = ofMap(StartTime(mBeats[i]), mZoomStart / sampleLength, mZoomEnd / sampleLength, 0, 1);
            float dur = ofMap(mBeats[i].mDuration, 0, (mZoomEnd - mZoomStart) / sampleLength, 0, 1);
            float end = pos + dur;
            if ((pos >= 0 && pos <= 1) || (end >= 0 && end <= 1))
            {
               pos = ofClamp(pos, 0, 1);
               end = ofClamp(end, 0, 1);
               ofRect(mBufferW * pos, mBufferH - BEAT_ZONE_HEIGHT, mBufferW * (end - pos), BEAT_ZONE_HEIGHT);
            }
         }

         for (int i = 0; i < mTatums.size(); ++i)
         {
            ofSetColor(0, 255, 255, ((i + 1) % 2) * 20);
            float pos = ofMap(StartTime(mTatums[i]), mZoomStart / sampleLength, mZoomEnd / sampleLength, 0, 1);
            float dur = ofMap(mTatums[i].mDuration, 0, (mZoomEnd - mZoomStart) / sampleLength, 0, 1);
            float end = pos + dur;
            if ((pos >= 0 && pos <= 1) || (end >= 0 && end <= 1))
            {
               pos = ofClamp(pos, 0, 1);
               end = ofClamp(end, 0, 1);
               ofRect(mBufferW * pos, MEASURE_ZONE_HEIGHT, mBufferW * (end - pos), mBufferH - MEASURE_ZONE_HEIGHT - BEAT_ZONE_HEIGHT);
            }
         }
      }

      ofPopStyle();
      ofPopMatrix();

      mClipStartSlider->Draw();
      mClipEndSlider->Draw();
      mZoomStartSlider->Draw();
      mZoomEndSlider->Draw();
      mNumBarsSlider->Draw();
      mOffsetSlider->Draw();
      mWriteButton->Draw();
      mDoubleLengthButton->Draw();
      mHalveLengthButton->Draw();
      if (mSample)
         DrawTextNormal(ofToString(mSample->GetPlayPosition()), 335, 50);
   }

   ofPushMatrix();
   ofPushStyle();
   ofFill();
   ofTranslate(mBufferX, mRemixBufferY);
   int sourceSampleLength = 1;
   if (mSample != nullptr)
      sourceSampleLength = MAX(1, mSample->LengthInSamples());
   ofPushMatrix();
   int count = 0;
   int i = 0;
   for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter, ++i)
   {
      float dur = ofMap((*iter)->mDuration * sourceSampleLength, mRemixZoomStart, mRemixZoomEnd, 0, 1);

      float width = MAX(0.0f, mBufferW * dur);
      DrawAudioBuffer(width, mBufferH, mSample->Data(), StartTime(*(*iter)) * sourceSampleLength, (StartTime(*(*iter)) + (*iter)->mDuration) * sourceSampleLength, -1);

      if ((*iter)->mType == kBlok_Bar)
      {
         ofSetColor(255, 0, (i % 2) * 255, 60);
         ofRect(0, 0, width, MEASURE_ZONE_HEIGHT);
      }
      if ((*iter)->mType == kBlok_Beat)
      {
         ofSetColor(255, 255, (i % 2) * 255, 60);
         ofRect(0, mBufferH - BEAT_ZONE_HEIGHT, width, BEAT_ZONE_HEIGHT);
      }
      if ((*iter)->mType == kBlok_Tatum)
      {
         ofSetColor(0, 255, 255, ((i + 1) % 2) * 20);
         ofRect(0, MEASURE_ZONE_HEIGHT, width, mBufferH - MEASURE_ZONE_HEIGHT - BEAT_ZONE_HEIGHT);
      }

      //ofSetColor(255,0,((count/8)%2)*255,255);
      //ofRect(0,mBufferH-30,width,30);

      if ((*iter)->mType == kBlok_Bar)
         count = (count / 8 + 1) * 8;
      if ((*iter)->mType == kBlok_Beat)
         count += 2;
      if ((*iter)->mType == kBlok_Tatum)
         count += 1;

      ofTranslate(width, 0);

      if (mHeldBlok && mLastRemovedRemixBlokIdx != -1 && mLastRemovedRemixBlokIdx - 1 == i)
      {
         ofPushStyle();
         ofSetLineWidth(5);
         ofSetColor(0, 255, 255, 200);
         ofLine(0, 0, 0, mBufferH);
         ofPopStyle();
      }
   }
   ofPopMatrix();
   if (mPlayRemix)
   {
      ofSetColor(0, 255, 0);
      float pos = mBufferW * ofMap(mRemixPlayhead, mRemixZoomStart, mRemixZoomEnd, 0, 1);
      ofLine(pos, 0, pos, mBufferH);
   }
   ofPopStyle();
   ofPopMatrix();

   if (mDrawSources)
   {
      ofPushStyle();
      ofSetColor(255, 200, 0, 200);
      float posRemix = 0;
      for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter)
      {
         float dur = ofMap((*iter)->mDuration * sourceSampleLength, mRemixZoomStart, mRemixZoomEnd, 0, 1);

         posRemix += dur / 2;

         float posSrc = ofMap(StartTime(*(*iter)), mZoomStart / sourceSampleLength, mZoomEnd / sourceSampleLength, 0, 1);
         float durSrc = ofMap((*iter)->mDuration, 0, (mZoomEnd - mZoomStart) / sourceSampleLength, 0, 1);
         posSrc += durSrc / 2;
         posSrc = ofClamp(posSrc, 0, 1);
         ofLine(mBufferX + mBufferW * posRemix, mRemixBufferY, mBufferX + mBufferW * posSrc, mBufferY + mBufferH);

         posRemix += dur / 2;
      }
      ofPopStyle();
   }

   if (mHeldBlok && mSample != nullptr)
   {
      ofPushMatrix();
      ofTranslate(mMouseX - mGrabOffsetX, mMouseY - mGrabOffsetY);

      int sampleLength = MAX(1, mSample->LengthInSamples());
      float dur = ofMap(mHeldBlok->mDuration, 0, (mZoomEnd - mZoomStart) / sampleLength, 0, 1);

      float width = mBufferW * dur;
      DrawAudioBuffer(width, mBufferH, mSample->Data(), StartTime(*mHeldBlok) * sampleLength, (StartTime(*mHeldBlok) + mHeldBlok->mDuration) * sampleLength, -1);

      ofPopMatrix();

      if (mMouseY > mRemixBufferY && mMouseY < mRemixBufferY + mBufferH)
      {
         ofPushStyle();
         ofSetColor(255, 255, 0);
         int insertIndex;
         float insertPos = GetInsertPosition(insertIndex);
         ofLine(insertPos, mRemixBufferY, insertPos, mRemixBufferY + mBufferH);
         ofPopStyle();
      }
   }
}

float BeatBloks::StartTime(const BeatBloks::Blok& blok)
{
   float numSamples = mSample ? mSample->LengthInSamples() : 1;
   return blok.mStartTime + mOffset / numSamples;
}

float BeatBloks::GetInsertPosition(int& insertIndex)
{
   float insertPos = mBufferX;
   int sourceSampleLength = mSample->LengthInSamples();
   int i = 0;
   for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter, ++i)
   {
      float dur = ofMap((*iter)->mDuration * sourceSampleLength, mRemixZoomStart, mRemixZoomEnd, 0, 1);

      float width = MAX(0.0f, mBufferW * dur);

      if (mMouseX < insertPos + width / 2)
      {
         insertIndex = i;
         return insertPos;
      }

      insertPos += width;
   }
   insertIndex = -1;
   return insertPos;
}

BeatBloks::Blok* BeatBloks::RemoveBlokAt(int x)
{
   int sourceSampleLength = mSample->LengthInSamples();
   float pos = mBufferX;
   int i = 0;
   for (auto iter = mRemixBloks.begin(); iter != mRemixBloks.end(); ++iter, ++i)
   {
      float dur = ofMap((*iter)->mDuration * sourceSampleLength, mRemixZoomStart, mRemixZoomEnd, 0, 1);

      float width = MAX(0.0f, mBufferW * dur);

      if (mMouseX < pos + width)
      {
         Blok* blok = (*iter);
         mRemixBloks.erase(iter);
         mLastRemovedRemixBlokIdx = i;
         return blok;
      }

      pos += width;
   }
   return nullptr;
}

void BeatBloks::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mPlayCheckbox)
   {
      if (mSample)
         mSample->Reset();
      TheTransport->SetMeasureTime(0);
   }
   if (checkbox == mPlayRemixCheckbox)
   {
      mRemixPlayhead = 0;
   }
}

void BeatBloks::GetModuleDimensions(float& width, float& height)
{
   width = 910;
   height = mRemixBufferY + mBufferH + 45;
}

void BeatBloks::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mClipStartSlider)
   {
      if (mSample && mClipStart > mSample->LengthInSamples())
         mClipStart = mSample->LengthInSamples();
   }
   if (slider == mClipEndSlider)
   {
      if (mSample && mClipEnd > mSample->LengthInSamples())
         mClipEnd = mSample->LengthInSamples();
   }
   if (slider == mZoomStartSlider)
   {
      if (mSample && mZoomStart > mSample->LengthInSamples())
         mZoomStart = mSample->LengthInSamples();
      UpdateZoomExtents();
   }
   if (slider == mZoomEndSlider)
   {
      if (mSample && mZoomEnd > mSample->LengthInSamples())
         mZoomEnd = mSample->LengthInSamples();
      UpdateZoomExtents();
   }
   if (slider == mRemixZoomStartSlider)
   {
      mRemixZoomStart = 0;
   }
}

void BeatBloks::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void BeatBloks::PlayNote(NoteMessage note)
{
   if (mSample)
   {
      mPlay = false;
      if (note.pitch == 16)
      {
         mSample->Reset();
      }
      else if (note.pitch >= 0 && note.pitch < 16 && note.velocity > 0)
      {
         int slice = (note.pitch / 8) * 8 + 7 - (note.pitch % 8);
         int barLength = (mClipEnd - mClipStart) / mNumBars;
         int position = (barLength / 4) * slice + mClipStart;
         mSample->Play(note.time, 1, position);
      }
   }
}

void BeatBloks::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BeatBloks::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

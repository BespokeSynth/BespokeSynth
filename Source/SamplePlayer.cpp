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
/*
  ==============================================================================

    SamplePlayer.cpp
    Created: 19 Oct 2017 10:10:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SamplePlayer.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "Scale.h"
#include "UIControlMacros.h"
#include "UserPrefs.h"

#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_formats/juce_audio_formats.h"

using namespace juce;

namespace
{
   const int kRecordingChunkSize = 48000 * 5;
   const int kMinRecordingChunks = 2;
};

SamplePlayer::SamplePlayer()
: IAudioProcessor(gBufferSize)
, mVolume(1)
, mVolumeSlider(nullptr)
, mSpeed(1)
, mSpeedSlider(nullptr)
, mSample(nullptr)
, mPlay(false)
, mLoop(false)
, mRecord(false)
, mLoopCheckbox(nullptr)
, mDrawBuffer(0)
, mPlayButton(nullptr)
, mPauseButton(nullptr)
, mStopButton(nullptr)
, mDownloadYoutubeButton(nullptr)
, mLoadFileButton(nullptr)
, mScrubbingSample(false)
, mOscWheelGrabbed(false)
, mOscWheelSpeed(0)
, mPlaySpeed(1)
, mWidth(608)
, mHeight(150)
, mNoteInputBuffer(this)
, mAdsr(10,1,1,10)
, mZoomLevel(1)
, mZoomOffset(0)
, mActiveCuePointIndex(0)
, mHoveredCuePointIndex(-1)
, mSetCuePoint(false)
, mSelectPlayedCuePoint(false)
, mRecentPlayedCuePoint(-1)
, mShowGrid(false)
, mRunningProcessType(RunningProcessType::None)
, mRunningProcess(nullptr)
, mOnRunningProcessComplete(nullptr)
, mIsLoadingSample(false)
, mLastOutputSample(1)
, mSwitchAndRampVal(1)
, mDoRecording(false)
, mRecordingLength(0)
, mRecordingAppendMode(false)
, mRecordAsClips(false)
, mRecordAsClipsCueIndex(0)
{
   mYoutubeSearch[0] = 0;
}

void SamplePlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mVolumeSlider, "volume",&mVolume,0,2); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mSpeedSlider,"speed",&mSpeed,-2,2); UIBLOCK_SHIFTRIGHT();
   BUTTON(mTrimToZoomButton, "trim"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mDownloadYoutubeButton,"youtube"); UIBLOCK_NEWLINE();
   TEXTENTRY(mDownloadYoutubeSearch,"yt:",30,mYoutubeSearch); UIBLOCK_NEWLINE();
   mDownloadYoutubeSearch->DrawLabel(true);
   mDownloadYoutubeSearch->SetRequireEnter(true);
   BUTTON_STYLE(mPlayButton,"play",ButtonDisplayStyle::kPlay); UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mPauseButton,"pause",ButtonDisplayStyle::kPause); UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mStopButton,"stop",ButtonDisplayStyle::kStop); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLoopCheckbox,"loop",&mLoop); UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(30);
   BUTTON(mLoadFileButton,"load"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveFileButton,"save"); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mRecordCheckbox,"record",&mRecord);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER_DIGITS(mCuePointStartSlider, "cue start", &mSampleCuePoints[0].startSeconds, 0, 100, 3);
   FLOATSLIDER_DIGITS(mCuePointLengthSlider, "cue len", &mSampleCuePoints[0].lengthSeconds, 0, 100, 3);
   FLOATSLIDER(mCuePointSpeedSlider, "cue speed", &mSampleCuePoints[0].speed, 0, 2);
   UIBLOCK_NEWCOLUMN();
   DROPDOWN(mCuePointSelector, "cuepoint", &mActiveCuePointIndex, 40);
   BUTTON(mPlayCurrentCuePointButton, "play cue");
   CHECKBOX(mCuePointStopCheckbox, "cue stop", &mSampleCuePoints[0].stopOnNoteOff);
   UIBLOCK_NEWCOLUMN();
   CHECKBOX(mSelectPlayedCuePointCheckbox, "select played", &mSelectPlayedCuePoint);
   CHECKBOX(mSetCuePointCheckbox, "click sets cue", &mSetCuePoint);
   BUTTON(mAutoSlice4, "4"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mAutoSlice8, "8"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mAutoSlice16, "16"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mAutoSlice32, "32");
   UIBLOCK_SHIFTX(-45);
   UIBLOCK_NEWCOLUMN();
   CHECKBOX(mShowGridCheckbox, "show grid", &mShowGrid);
   CHECKBOX(mRecordingAppendModeCheckbox, "append to rec", &mRecordingAppendMode)
   CHECKBOX(mRecordAsClipsCheckbox, "record as clips", &mRecordAsClips);
   ENDUIBLOCK0();

   UIBLOCK(0, 65);
   for (size_t i = 0; i < mSearchResultButtons.size(); ++i)
   {
      BUTTON(mSearchResultButtons[i], ("searchresult" + ofToString(i)).c_str());
      mSearchResultButtons[i]->SetShowing(false);
   }
   ENDUIBLOCK0();
   
   mPlayHoveredClipButton = new ClickButton(this, "playhovered", -1, -1, ButtonDisplayStyle::kPlay);
   mGrabHoveredClipButton = new ClickButton(this, "grabhovered", -1, -1, ButtonDisplayStyle::kGrabSample);
   mPlayHoveredClipButton->SetShowing(false);
   mGrabHoveredClipButton->SetShowing(false);

   for (int i = 0; i < (int)mSampleCuePoints.size(); ++i)
      mCuePointSelector->AddLabel(ofToString(i).c_str(), i);
   
   AddChild(&mRecordGate);
   mRecordGate.SetPosition(mRecordAsClipsCheckbox->GetRect().getMaxX()+3,-1);
   mRecordGate.SetEnabled(mRecordAsClips);
   mRecordGate.SetAttack(1);
   mRecordGate.SetRelease(100);
   mRecordGate.SetName("gate");
   mRecordGate.CreateUIControls();
}

SamplePlayer::~SamplePlayer()
{
   if (mOwnsSample)
      delete mSample;
   for (size_t i=0; i<mRecordChunks.size(); ++i)
      delete mRecordChunks[i];
}

void SamplePlayer::Init()
{
   IDrawableModule::Init();
   
   if (OSCReceiver::connect(12345))
      OSCReceiver::addListener(this);
}

void SamplePlayer::Poll()
{
   IDrawableModule::Poll();
   
   const juce::String& clipboard = TheSynth->GetTextFromClipboard();
   if (clipboard.contains("youtube"))
   {
      juce::String clipId = clipboard.substring(clipboard.indexOf("v=")+2, clipboard.length());
      mYoutubeId = clipId.toStdString();
      mDownloadYoutubeButton->SetShowing(true);
   }
   else if (clipboard.contains("youtu.be"))
   {
      std::vector<std::string> tokens = ofSplitString(clipboard.toStdString(), "/");
      if (tokens.size() > 0)
      {
         mYoutubeId = tokens[tokens.size() - 1];
         mDownloadYoutubeButton->SetShowing(true);
      }
   }
   else
   {
      mYoutubeId = "";
      mDownloadYoutubeButton->SetShowing(false);
   }

   if (mRunningProcess != nullptr)
   {
      if (mModuleSaveData.GetBool("show_youtube_process_output"))
      {
         char buffer[512];
         auto num = mRunningProcess->readProcessOutput(buffer, sizeof(buffer));

         if (num > 0)
         {
            MemoryOutputStream result;
            result.write(buffer, (size_t)num);
            ofLog() << result.toString();
         }
      }

      if (mRunningProcessType == RunningProcessType::SearchYoutube)
      {
         std::string tempPath = ofToDataPath("youtube_temp");
         auto dir = juce::File(tempPath);
         if (dir.exists())
         {
            Array<juce::File> results;
            dir.findChildFiles(results, juce::File::findFiles, false, "*.info.json");
            if (results.size() != mYoutubeSearchResults.size())
            {
               for (auto& result : results)
               {
                  std::string file = result.getFileName().toStdString();
                  ofStringReplace(file, ".info.json", "");
                  std::vector<std::string> tokens = ofSplitString(file, "#");
                  if (tokens.size() >= 3)
                  {
                     std::string lengthStr = tokens[tokens.size() - 3];
                     std::string id = tokens[tokens.size() - 2];
                     std::string channel = tokens[tokens.size() - 1];
                     bool found = false;
                     for (auto& existing : mYoutubeSearchResults)
                     {
                        if (existing.youtubeId == id)
                           found = true;
                     }
                     if (!found)
                     {
                        YoutubeSearchResult resultToAdd;
                        std::string name = "";
                        for (size_t i = 0; i < tokens.size() - 3; ++i)
                           name += tokens[i];
                        resultToAdd.name = name;
                        resultToAdd.channel = channel;
                        resultToAdd.lengthSeconds = ofToFloat(lengthStr);
                        resultToAdd.youtubeId = id;
                        mYoutubeSearchResults.push_back(resultToAdd);
                     }
                  }
               }
            }
         }
      }

      if (!mRunningProcess->isRunning())
      {
         ofLog() << mRunningProcess->readAllProcessOutput();
         delete mRunningProcess;
         mRunningProcess = nullptr;
         if (mOnRunningProcessComplete != nullptr)
            mOnRunningProcessComplete();
      }
   }

   if (mRecentPlayedCuePoint != -1)
   {
      int index = mRecentPlayedCuePoint;
      mRecentPlayedCuePoint = -1;

      if (index >= 0 && index < (int)mSampleCuePoints.size())
      {
         mActiveCuePointIndex = index;
         UpdateActiveCuePoint();
      }
   }
   
   if (mDoRecording)
   {
      int chunkIndex = mRecordingLength / kRecordingChunkSize;
      if (chunkIndex >= (int)mRecordChunks.size() - 1)
      {
         mRecordChunks.push_back(new ChannelBuffer(kRecordingChunkSize));
         mRecordChunks[mRecordChunks.size()-1]->GetChannel(0); //set up buffer
      }
   }
}

void SamplePlayer::Process(double time)
{
   PROFILER(SamplePlayer);
   
   IAudioReceiver* target = GetTarget();
   
   //recording input gate processing
   bool gateWasOpen = false;
   bool gateIsOpen = false;
   if (mRecordAsClips)
   {
      gateWasOpen = mRecordGate.IsGateOpen();
      mRecordGate.ProcessAudio(time, GetBuffer());
      gateIsOpen = mRecordGate.IsGateOpen();
   }
   
   if (mDoRecording)
   {
      bool acceptInput = true;
      if (mRecordAsClips)
      {
         acceptInput = gateIsOpen;
         
         if (gateIsOpen && !gateWasOpen)
         {
            if (mRecordAsClipsCueIndex < (int)mSampleCuePoints.size())
            {
               SetCuePoint(mRecordAsClipsCueIndex, float(mRecordingLength) / gSampleRate, 0, 1);
            }
         }
         
         if (!gateIsOpen && gateWasOpen)
         {
            if (mRecordAsClipsCueIndex < (int)mSampleCuePoints.size())
               mSampleCuePoints[mRecordAsClipsCueIndex].lengthSeconds = (float(mRecordingLength) / gSampleRate) - mSampleCuePoints[mRecordAsClipsCueIndex].startSeconds;
            mRecordingLength += 200; //add silence gap
            ++mRecordAsClipsCueIndex;
         }
      }
      
      if (acceptInput)
      {
         for (int i=0; i<GetBuffer()->BufferSize(); ++i)
         {
            int chunkIndex = mRecordingLength / kRecordingChunkSize;
            int chunkPos = mRecordingLength % kRecordingChunkSize;
            mRecordChunks[chunkIndex]->SetNumActiveChannels(GetBuffer()->NumActiveChannels());
            for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
               mRecordChunks[chunkIndex]->GetChannel(ch)[chunkPos] = GetBuffer()->GetChannel(ch)[i];
            ++mRecordingLength;
         }
      }
   }

   if (mEnabled && target != nullptr && mSample != nullptr)
   {
      mNoteInputBuffer.Process(time);
      
      ComputeSliders(0);
      SyncBuffers(mSample->NumChannels());
      
      int bufferSize = target->GetBuffer()->BufferSize();
      assert(bufferSize == gBufferSize);
      
      float volSq = mVolume * mVolume;
      
      const float kBlendSpeed = 1;
      if (mOscWheelGrabbed)
      {
         mPlaySpeed = ofLerp(mPlaySpeed, mOscWheelSpeed, kBlendSpeed);
         mPlaySpeed = ofClamp(mPlaySpeed, -5, 5);
      }
      else
      {
         mPlaySpeed = ofLerp(mPlaySpeed, mSpeed * mCuePointSpeed, kBlendSpeed);
      }
      mSample->SetRate(mPlaySpeed);
      
      gWorkChannelBuffer.SetNumActiveChannels(mSample->NumChannels());
      mLastOutputSample.SetNumActiveChannels(mSample->NumChannels());
      mSwitchAndRampVal.SetNumActiveChannels(mSample->NumChannels());

      if (mPlay)
      {
         if (mSample->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
         {
            for (int ch = 0; ch < gWorkChannelBuffer.NumActiveChannels(); ++ch)
            {
               for (int i = 0; i < bufferSize; ++i)
                  gWorkChannelBuffer.GetChannel(ch)[i] *= volSq * mAdsr.Value(time + i * gInvSampleRateMs);
            }
         }
         else
         {
            gWorkChannelBuffer.Clear();
            mPlay = false;
            mSample->SetPlayPosition(0);
            mAdsr.Stop(time);
         }
      }
      else
      {
         gWorkChannelBuffer.Clear();
      }

      for (int ch = 0; ch < gWorkChannelBuffer.NumActiveChannels(); ++ch)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            gWorkChannelBuffer.GetChannel(ch)[i] += mSwitchAndRampVal.GetChannel(ch)[0];
            mSwitchAndRampVal.GetChannel(ch)[0] *= .999f;
            if (mSwitchAndRampVal.GetChannel(ch)[0] < .0001f && mSwitchAndRampVal.GetChannel(ch)[0] > -.0001f)
               mSwitchAndRampVal.GetChannel(ch)[0] = 0;
         }

         Add(target->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
         mLastOutputSample.GetChannel(ch)[0] = gWorkChannelBuffer.GetChannel(ch)[bufferSize-1];
      }
   }
   
   GetBuffer()->Reset();
}

void SamplePlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationParameters modulation /*= ModulationParameters()*/)
{
   if (!mEnabled)
      return;

   if (mSelectPlayedCuePoint)
      mRecentPlayedCuePoint = pitch;

   if (!NoteInputBuffer::IsTimeWithinFrame(time) && GetTarget() && mSample)
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0 && mSample != nullptr)
      PlayCuePoint(time, pitch, velocity, modulation.pitchBend ? exp2(modulation.pitchBend->GetValue(0)) : 1, modulation.modWheel ? modulation.modWheel->GetValue(0) : 0);

   if (velocity == 0 && mStopOnNoteOff)
   {
      mAdsr.Stop(time);
   }
}

void SamplePlayer::PlayCuePoint(double time, int index, int velocity, float speedMult, float startOffsetSeconds)
{
   if (mSample != nullptr)
   {
      float startSeconds, lengthSeconds, speed;
      GetPlayInfoForPitch(index, startSeconds, lengthSeconds, speed, mStopOnNoteOff);
      mSample->SetPlayPosition(((gTime - time) / 1000 + startSeconds + startOffsetSeconds) * gSampleRate * mSample->GetSampleRateRatio());
      mCuePointSpeed = speed * speedMult;
      mPlay = true;
      mAdsr.Clear();
      mAdsr.Start(time, velocity / 127.0f);
      if (lengthSeconds > 0)
         mAdsr.Stop(time + lengthSeconds * 1000 / speed);
      SwitchAndRamp();
   }
}

void SamplePlayer::SwitchAndRamp()
{
   for (int ch = 0; ch < mSwitchAndRampVal.NumActiveChannels(); ++ch)
      mSwitchAndRampVal.GetChannel(ch)[0] = mLastOutputSample.GetChannel(ch)[0];
}

void SamplePlayer::DropdownClicked(DropdownList* list)
{
}

void SamplePlayer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mCuePointSelector)
      UpdateActiveCuePoint();
}

void SamplePlayer::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   
}

void SamplePlayer::UpdateActiveCuePoint()
{
   if (mActiveCuePointIndex >= 0 && mActiveCuePointIndex < (int)mSampleCuePoints.size())
   {
      mCuePointStartSlider->SetVar(&mSampleCuePoints[mActiveCuePointIndex].startSeconds);
      mCuePointLengthSlider->SetVar(&mSampleCuePoints[mActiveCuePointIndex].lengthSeconds);
      mCuePointSpeedSlider->SetVar(&mSampleCuePoints[mActiveCuePointIndex].speed);
      mCuePointStopCheckbox->SetVar(&mSampleCuePoints[mActiveCuePointIndex].stopOnNoteOff);
   }
}

void SamplePlayer::AutoSlice(int slices)
{
   float sliceLengthSeconds = GetLengthInSeconds() / slices;
   for (int i = 0; i < (int)mSampleCuePoints.size(); ++i)
   {
      if (i < slices)
      {
         mSampleCuePoints[i].startSeconds = sliceLengthSeconds *i;
         mSampleCuePoints[i].lengthSeconds = sliceLengthSeconds;
         mSampleCuePoints[i].speed = 1;
      }
      else
      {
         mSampleCuePoints[i].startSeconds = 0;
         mSampleCuePoints[i].lengthSeconds = 0;
      }
   }
}

void SamplePlayer::FilesDropped(std::vector<std::string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   UpdateSample(sample, true);
}

void SamplePlayer::SampleDropped(int x, int y, Sample* sample)
{
   if (gHoveredUIControl == nullptr)   //avoid problem of grabbing a clip via the clip grab button and immediately dropping it onto this sampleplayer by accident
   {
      Sample* copy = new Sample();
      copy->CopyFrom(sample);
      UpdateSample(copy, true);
   }
}

void SamplePlayer::UpdateSample(Sample* sample, bool ownsSample)
{
   Sample* oldSamplePtr = mSample;

   float lengthSeconds = sample->LengthInSamples() / (gSampleRate * sample->GetSampleRateRatio());
   mCuePointStartSlider->SetExtents(0, lengthSeconds);
   mCuePointLengthSlider->SetExtents(0, lengthSeconds);
   
   sample->SetPlayPosition(0);
   sample->SetLooping(mLoop);
   sample->SetRate(mSpeed);
   mSample = sample;
   mVolume = 1;
   mPlay = false;
   mOwnsSample = ownsSample;   
   mZoomLevel = 1;
   mZoomOffset = 0;
   mRecordingLength = 0;

   mErrorString = "";

   if (mOwnsSample)
      delete oldSamplePtr;

   mIsLoadingSample = true;
}

void SamplePlayer::ButtonClicked(ClickButton *button)
{
   if (button == mPlayButton && mSample != nullptr)
   {
      if (mRecord == false)
      {
         mCuePointSpeed = 1;
         mStopOnNoteOff = false;
         mPlay = true;
         mAdsr.Clear();
         mAdsr.Start(gTime + gBufferSize*gInvSampleRateMs, 1);
      }
   }
   if (button == mPauseButton && mSample != nullptr)
   {
      mPlay = false;
      SwitchAndRamp();
   }
   if (button == mStopButton)
   {
      if (mRecord)
      {
         StopRecording();
      }
      else if (mSample != nullptr)
      {
         mPlay = false;
         mSample->SetPlayPosition(0);
         SwitchAndRamp();
      }
   }
   if (button == mDownloadYoutubeButton)
      DownloadYoutube("https://www.youtube.com/watch?v="+mYoutubeId, mYoutubeId);
   if (button == mLoadFileButton)
      LoadFile();
   if (button == mSaveFileButton)
      SaveFile();
   if (button == mTrimToZoomButton && mSample != nullptr)
   {
      for (auto& cuePoint : mSampleCuePoints)
         cuePoint.startSeconds = MAX(0, cuePoint.startSeconds - GetZoomStartSeconds());

      Sample* sample = new Sample();
      sample->Create(GetZoomEndSample() - GetZoomStartSample());
      sample->Data()->SetNumActiveChannels(mSample->NumChannels());
      for (int ch = 0; ch < mSample->NumChannels(); ++ch)
      {
         float* sampleData = sample->Data()->GetChannel(ch);
         for (int i = 0; i < sample->LengthInSamples(); ++i)
            sampleData[i] = mSample->Data()->GetChannel(ch)[i + GetZoomStartSample()];
      }
      sample->SetName(mSample->Name());
      UpdateSample(sample, true);
   }

   for (size_t i = 0; i < mSearchResultButtons.size(); ++i)
   {
      if (button == mSearchResultButtons[i])
      {
         if (i < mYoutubeSearchResults.size())
         {
            DownloadYoutube("https://www.youtube.com/watch?v=" + mYoutubeSearchResults[i].youtubeId, mYoutubeSearchResults[i].name);

            mYoutubeSearchResults.clear();
            break;
         }
      }
   }

   if (button == mPlayCurrentCuePointButton)
      PlayCuePoint(gTime, mActiveCuePointIndex, 127, 1, 0);

   if (button == mAutoSlice4)
      AutoSlice(4);
   if (button == mAutoSlice8)
      AutoSlice(8);
   if (button == mAutoSlice16)
      AutoSlice(16);
   if (button == mAutoSlice32)
      AutoSlice(32);
   
   if (button == mPlayHoveredClipButton)
      PlayCuePoint(gTime, mHoveredCuePointIndex, 127, 1, 0);
   if (button == mGrabHoveredClipButton)
   {
      ChannelBuffer* data = GetCueSampleData(mHoveredCuePointIndex);
      TheSynth->GrabSample(data, mSample->Name(), false);
      delete data;
   }
}

void SamplePlayer::TextEntryComplete(TextEntry* entry)
{
   if (entry == mDownloadYoutubeSearch)
   {
      SearchYoutube(mYoutubeSearch);
      //youtube-dl "ytsearch5:duck quack sound effect" --no-playlist --write-info-json --skip-download -o "test/%(title)s [len %(duration)s] [id %(id)s].%(ext)s"
   }
}

void SamplePlayer::DownloadYoutube(std::string url, std::string title)
{
   mPlay = false;
   if (mSample)
      mSample->SetPlayPosition(0);

   const char* tempDownloadName = "youtube.m4a";
   {
      auto file = juce::File(ofToDataPath(tempDownloadName));
      if (file.existsAsFile())
         file.deleteFile();
   }

   const char* tempConvertedName = "youtube.wav";
   {
      auto file = juce::File(ofToDataPath(tempConvertedName));
      if (file.existsAsFile())
         file.deleteFile();
   }
   
   StringArray args;
   args.add(UserPrefs.youtube_dl_path.Get());
   args.add(url);
   args.add("--extract-audio");
   args.add("--audio-format");
   args.add("wav");
   args.add("--audio-quality");
   args.add("0");
   args.add("--no-progress");
   args.add("--ffmpeg-location");
   args.add(UserPrefs.ffmpeg_path.Get());
   args.add("-o");
   args.add(ofToDataPath(tempDownloadName));
   
   mRunningProcessType = RunningProcessType::DownloadYoutube;

   mOnRunningProcessComplete = [this, tempConvertedName, title] { OnYoutubeDownloadComplete(tempConvertedName, title); };

   RunProcess(args);
}

void SamplePlayer::OnYoutubeDownloadComplete(std::string filename, std::string title)
{
   if (juce::File(ofToDataPath(filename)).existsAsFile())
   {
      Sample* sample = new Sample();
      sample->Read(ofToDataPath(filename).c_str(), false, Sample::ReadType::Async);
      sample->SetName(title);
      UpdateSample(sample, true);
   }
   else
   {
      UpdateSample(new Sample(), true);
      mErrorString = "couldn't download sample. do you have youtube-dl and ffmpeg installed,\nwith their paths set in userprefs.json?";
   }
}

void SamplePlayer::RunProcess(const StringArray& args)
{
   std::string command = "";
   for (auto& arg : args)
      command += arg.toStdString() + " ";
   ofLog() << "running " << command;
   if (mRunningProcess)
      mRunningProcess->kill();
   delete mRunningProcess;
   mRunningProcess = new ChildProcess();
   bool success = mRunningProcess->start(args);
   if (!success)
      ofLog() << "error running process from bespoke";
}

void SamplePlayer::SearchYoutube(std::string searchTerm)
{
   std::string tempPath = ofToDataPath("youtube_temp");
   auto dir = juce::File(tempPath);
   if (dir.exists())
      dir.deleteRecursively();
   dir.createDirectory();
   mYoutubeSearchResults.clear();

   StringArray args;
   args.add(UserPrefs.youtube_dl_path.Get());
   args.add("ytsearch"+ofToString(kMaxYoutubeSearchResults)+":"+searchTerm);
   args.add("--no-playlist");
   args.add("--write-info-json");
   args.add("--skip-download");
   args.add("-o");
   args.add(tempPath+"/%(title)s#%(duration)s#%(id)s#%(uploader)s.%(ext)s");

   mRunningProcessType = RunningProcessType::SearchYoutube;

   double searchTime = gTime;
   mOnRunningProcessComplete = [this, searchTerm, searchTime] { OnYoutubeSearchComplete(searchTerm, searchTime); };

   RunProcess(args);
}

void SamplePlayer::OnYoutubeSearchComplete(std::string searchTerm, double searchTime)
{
   if (mYoutubeSearchResults.size() == 0)
   {
      if (gTime - searchTime < 500)
      {
         mErrorString = "couldn't search. do you have youtube-dl installed, with its path set in userprefs.json?";
      }
      else
      {
         mErrorString = "zero results found for " + searchTerm;
      }
   }
}

void SamplePlayer::LoadFile()
{
   FileChooser chooser("Load sample", File(ofToDataPath("samples")),
                       TheSynth->GetAudioFormatManager().getWildcardForAllFormats(), true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
   if (chooser.browseForFileToOpen())
   {
      auto file = chooser.getResult();

      Sample* sample = new Sample();
      if (file.existsAsFile())
         sample->Read(file.getFullPathName().toStdString().c_str());
      UpdateSample(sample, true);
   }
}

void SamplePlayer::SaveFile()
{
   FileChooser chooser("Save sample", File(ofToDataPath("samples")),
                       "*.wav", true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
   if (chooser.browseForFileToSave(true))
   {
      auto file = chooser.getResult();
      Sample::WriteDataToFile(file.getFullPathName().toStdString().c_str(), mSample->Data(), mSample->LengthInSamples());
   }
}

void SamplePlayer::FillData(std::vector<float> data)
{
   Sample* sample = new Sample();
   sample->Create((int)data.size());
   float* sampleData = sample->Data()->GetChannel(0);
   for (size_t i = 0; i < data.size(); ++i)
      sampleData[i] = data[i];
   UpdateSample(sample, true);
}

void SamplePlayer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;

   if (mYoutubeSearchResults.size() > 0)
      return;
   
   if (y > 60 && y < mHeight - 20 && mSample != nullptr && gHoveredUIControl == nullptr)
   {
      SwitchAndRamp();
      mCuePointSpeed = 1;
      mStopOnNoteOff = false;
      mPlay = true;
      mAdsr.Clear();
      mAdsr.Start(gTime + gBufferSizeMs, 1);
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
      mScrubbingSample = true;

      if (mSetCuePoint)
         SetCuePointForX(x);
   }
}

ChannelBuffer* SamplePlayer::GetCueSampleData(int cueIndex)
{
   float startSeconds, lengthSeconds, speed;
   bool stopOnNoteOff;
   GetPlayInfoForPitch(cueIndex, startSeconds, lengthSeconds, speed, stopOnNoteOff);
   if (lengthSeconds <= 0)
      lengthSeconds = 1;
   int startSamples = startSeconds * gSampleRate * mSample->GetSampleRateRatio();
   int lengthSamplesSrc = lengthSeconds * gSampleRate * mSample->GetSampleRateRatio();
   if (startSamples >= mSample->Data()->BufferSize())
      startSamples = mSample->Data()->BufferSize() - 1;
   if (startSamples + lengthSamplesSrc >= mSample->Data()->BufferSize())
      lengthSamplesSrc = mSample->Data()->BufferSize() - 1 - startSamples;
   int lengthSamplesDest = lengthSamplesSrc / speed / mSample->GetSampleRateRatio();
   ChannelBuffer* data = new ChannelBuffer(lengthSamplesDest);
   data->SetNumActiveChannels(mSample->Data()->NumActiveChannels());
   /*for (int ch = 0; ch < data->NumActiveChannels(); ++ch)
   {
      BufferCopy(data->GetChannel(ch), mSample->Data()->GetChannel(ch) + startSamples, lengthSamplesSrc);
   }*/

   for (int ch = 0; ch < data->NumActiveChannels(); ++ch)
   {
      for (int i = 0; i < lengthSamplesDest; ++i)
      {
         float offset = i * speed * mSample->GetSampleRateRatio();
         data->GetChannel(ch)[i] = GetInterpolatedSample(offset, mSample->Data()->GetChannel(ch) + startSamples, lengthSamplesSrc);
      }
   }
   
   return data;
}

bool SamplePlayer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   if (mScrubbingSample && mSample != nullptr)
   {
      SwitchAndRamp();
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
      mAdsr.Clear();
      mAdsr.Start(gTime + gBufferSizeMs, 1);

      if (mSetCuePoint)
         SetCuePointForX(x);
   }
   
   if (gHoveredUIControl == nullptr)    //make sure we don't update our hover while dragging a slider
   {
      mHoveredCuePointIndex = -1;
      if (y > 60 && y < mHeight - 20 && mSample != nullptr)
      {
         float seconds = GetSecondsForMouse(x);

         // find cue point closest to but not exceeding the cursor position
         int bestCuePointIndex = -1;
         float bestCuePointStart = 0.;
         for (size_t i = 0; i < mSampleCuePoints.size(); ++i)
         {
            float startSeconds = mSampleCuePoints[i].startSeconds;
            float lengthSeconds = mSampleCuePoints[i].lengthSeconds;

            if (lengthSeconds > 0.)
            {
               if (seconds >= startSeconds && seconds <= startSeconds + lengthSeconds &&
                   startSeconds > bestCuePointStart)
               {
                  bestCuePointIndex = i;
                  bestCuePointStart = startSeconds;
               }
            }
         }

         if (bestCuePointIndex != -1)
         {
            mHoveredCuePointIndex = bestCuePointIndex;
            mActiveCuePointIndex = bestCuePointIndex;
            UpdateActiveCuePoint();
         }
      }
   }

   return true;
}

void SamplePlayer::SetCuePointForX(float mouseX)
{
   mSampleCuePoints[mActiveCuePointIndex].startSeconds = GetPlayPositionForMouse(mouseX) / (gSampleRate * mSample->GetSampleRateRatio());
   mSampleCuePoints[mActiveCuePointIndex].speed = 1;
}

void SamplePlayer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mScrubbingSample = false;
}

float SamplePlayer::GetPlayPositionForMouse(float mouseX) const
{
   return ofMap(mouseX, 5, mWidth-5, GetZoomStartSample(), GetZoomEndSample(), true);
}

float SamplePlayer::GetSecondsForMouse(float mouseX) const
{
   return ofMap(mouseX, 5, mWidth-5, GetZoomStartSeconds(), GetZoomEndSeconds(), true);
}

void SamplePlayer::GetPlayInfoForPitch(int pitch, float& startSeconds, float& lengthSeconds, float& speed, bool& stopOnNoteOff) const
{
   if (pitch < mSampleCuePoints.size())
   {
      startSeconds = mSampleCuePoints[pitch].startSeconds;
      lengthSeconds = mSampleCuePoints[pitch].lengthSeconds;
      speed = mSampleCuePoints[pitch].speed;
      stopOnNoteOff = mSampleCuePoints[pitch].stopOnNoteOff;
   }
   else
   {
      startSeconds = 0;
      lengthSeconds = 0;
      speed = 1;
      stopOnNoteOff = false;
   }
}

void SamplePlayer::SetCuePoint(int pitch, float startSeconds, float lengthSeconds, float speed)
{
   if (pitch < mSampleCuePoints.size())
   {
      mSampleCuePoints[pitch].startSeconds = startSeconds;
      mSampleCuePoints[pitch].lengthSeconds = lengthSeconds;
      mSampleCuePoints[pitch].speed = speed;
   }
}

void SamplePlayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTrimToZoomButton->SetShowing(mZoomLevel != 1);
   
   mVolumeSlider->Draw();
   mSpeedSlider->Draw();
   mLoopCheckbox->Draw();
   mPlayButton->Draw();
   mPauseButton->Draw();
   mStopButton->Draw();
   mTrimToZoomButton->Draw();
   mDownloadYoutubeButton->Draw();
   mDownloadYoutubeSearch->Draw();
   mLoadFileButton->Draw();
   mSaveFileButton->Draw();
   mRecordCheckbox->Draw();
   mCuePointSelector->Draw();
   mSetCuePointCheckbox->Draw();
   mSelectPlayedCuePointCheckbox->Draw();
   mCuePointStartSlider->Draw();
   mCuePointLengthSlider->Draw();
   mCuePointSpeedSlider->Draw();
   mCuePointStopCheckbox->Draw();
   mPlayCurrentCuePointButton->Draw();
   mShowGridCheckbox->Draw();
   mAutoSlice4->Draw();
   mAutoSlice8->Draw();
   mAutoSlice16->Draw();
   mAutoSlice32->Draw();
   mRecordingAppendModeCheckbox->Draw();
   mRecordAsClipsCheckbox->Draw();
   mRecordGate.Draw();

   for (size_t i = 0; i < mSearchResultButtons.size(); ++i)
   {
      if (i < mYoutubeSearchResults.size())
      {
         mSearchResultButtons[i]->SetShowing(true);
         int minutes = int(mYoutubeSearchResults[i].lengthSeconds / 60);
         int secondsRemainder = int(mYoutubeSearchResults[i].lengthSeconds) % 60;
         std::string lengthStr = ofToString(minutes) + ":";
         if (secondsRemainder < 10)
            lengthStr += "0";
         lengthStr += ofToString(secondsRemainder);
         mSearchResultButtons[i]->SetLabel(("(" + lengthStr + ") " + mYoutubeSearchResults[i].name + "   [" + mYoutubeSearchResults[i].channel + "]").c_str());
      }
      else
      {
         mSearchResultButtons[i]->SetShowing(false);
      }
      mSearchResultButtons[i]->Draw();
   }

   ofPushMatrix();
   ofTranslate(5,58);
   float sampleWidth = mWidth - 10;
   if (mDoRecording)
   {
      ofSetColor(255, 0, 0, 100);
      ofRect(0, 0, mWidth - 10, mHeight - 65);

      ofPushMatrix();
      
      int numChunks = mRecordingLength / kRecordingChunkSize + 1;
      float chunkWidth = sampleWidth / numChunks;
      for (int i=0; i<numChunks; ++i)
      {
         DrawAudioBuffer(chunkWidth, mHeight - 65, mRecordChunks[i], 0, kRecordingChunkSize, -1);
         ofTranslate(chunkWidth, 0);
      }
      ofPopMatrix();
   }
   else if (mRunningProcess != nullptr || (mSample && mSample->IsSampleLoading()))
   {
      const int kNumDots = 8;
      const float kCircleRadius = 20;
      const float kDotRadius = 3;
      const float kSpinSpeed = .003f;
      ofPushStyle();
      ofFill();
      for (int i = 0; i < kNumDots; ++i)
      {
         float theta = float(i) / kNumDots * M_PI * 2 + gTime * kSpinSpeed;
         ofCircle(cos(theta) * kCircleRadius + (mWidth-10) * .5f, sin(theta) * kCircleRadius + (mHeight-65) * .5f, kDotRadius);
      }
      ofPopStyle();

      if (mSample && mSample->IsSampleLoading())
      {
         ofPushStyle();
         ofFill();
         ofSetColor(255, 255, 255, 50);
         ofRect(0, 0, (mWidth - 10) * mSample->GetSampleLoadProgress(), mHeight - 65);
         ofSetColor(40, 40, 40);
         DrawTextNormal("loading sample...", 10, 10, 10);
         ofPopStyle();
      }
   }
   else if (mYoutubeSearchResults.size() > 0)
   {
      //don't draw, buttons will draw instead
   }
   else if (mErrorString != "")
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 255, 255, 50);
      ofRect(0, 0, mWidth - 10, mHeight - 65);
      ofSetColor(220, 0, 0);
      DrawTextNormal(mErrorString, 10, 10, 10);
      ofPopStyle();
   }
   else if (mSample && mSample->LengthInSamples() > 0)
   {
      if (mIsLoadingSample && !mSample->IsSampleLoading())
      {
         mIsLoadingSample = false;
         mDrawBuffer.Resize(mSample->LengthInSamples());
         mDrawBuffer.CopyFrom(mSample->Data());
      }

      int playPosition = mSample->GetPlayPosition();
      if (mAdsr.Value(gTime) == 0)
         playPosition = -1;
      DrawAudioBuffer(sampleWidth, mHeight - 65, &mDrawBuffer, GetZoomStartSample(), GetZoomEndSample(), playPosition);
      
      ofPushStyle();
      ofFill();

      ofSetColor(255, 255, 255);
      DrawTextNormal(mSample->Name(), 5, 27);

      if (playPosition >= 0)
      {
         float x = ofMap(playPosition, GetZoomStartSample(), GetZoomEndSample(), 0, sampleWidth);
         DrawTextNormal(ofToString(playPosition / (gSampleRate * mSample->GetSampleRateRatio()), 1), x + 2, mHeight - 65, 11);
      }

      if (mShowGrid)
      {
         float lengthSeconds = GetZoomEndSeconds() - GetZoomStartSeconds();
         float lengthBeats = TheTransport->GetTempo() * (lengthSeconds / 60) / mSampleCuePoints[mActiveCuePointIndex].speed;
         if (lengthBeats < 30)
         {
            float alpha = ofMap(lengthBeats, 30, 28, 0, 200, true);
            ofSetColor(0, 255, 255, alpha);
            float secondsPerBeat = 60 / (TheTransport->GetTempo() / mSampleCuePoints[mActiveCuePointIndex].speed);
            float offset = mSampleCuePoints[mActiveCuePointIndex].startSeconds;
            float firstBeat = ceil((GetZoomStartSeconds() - offset) / secondsPerBeat);
            float firstBeatSeconds = firstBeat * secondsPerBeat + offset;
            for (int i = 0; i < ceil(lengthBeats); ++i)
            {
               float second = firstBeatSeconds + i * secondsPerBeat;
               float x = ofMap(second, GetZoomStartSeconds(), GetZoomEndSeconds(), 0, sampleWidth);
               ofLine(x, 0, x, mHeight - 65);
            }
         }
      }

      ofPopStyle();
   }
   else
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,255,255,50);
      ofRect(0, 0, mWidth-10, mHeight - 65);
      ofSetColor(40,40,40);
      DrawTextNormal("drag and drop a sample here...", 10, 10, 10);
      ofPopStyle();
   }
   
   if ((mSample && mSample->LengthInSamples() > 0) || mDoRecording)
   {
      ofPushStyle();
      ofFill();
      for (size_t i=0; i<mSampleCuePoints.size(); ++i)
      {
         if (mSampleCuePoints[i].lengthSeconds > 0 || mSampleCuePoints[i].startSeconds > 0)
         {
            float x = ofMap(mSampleCuePoints[i].startSeconds, GetZoomStartSeconds(), GetZoomEndSeconds(), 0, sampleWidth);
            float xEnd = ofMap(mSampleCuePoints[i].startSeconds + mSampleCuePoints[i].lengthSeconds, GetZoomStartSeconds(), GetZoomEndSeconds(), 0, sampleWidth);
            ofSetColor(0, 0, 0, 100);
            ofRect(x, 0, MAX((xEnd - x), 10), 10);
            ofRect(x, 0, 15, 10);
            ofSetColor(255, 255, 255);
            ofRect(x, 0, 1, 20, 1);
            if (i == mActiveCuePointIndex)
            {
               ofNoFill();
               ofRect(x, 0, 15, 10);
               ofFill();
            }
            DrawTextNormal(ofToString((int)i), x+2, 8, 11);
            
            if (i == mHoveredCuePointIndex)
            {
               ofSetColor(255,255,255,50);
               ofRect(x, 0, (xEnd-x), mHeight-65);
            }
         }
      }
      ofPopStyle();
   }
   
   ofPopMatrix();

   if (mZoomLevel != 1)
   {
      ofNoFill();
      ofRect(5, mHeight - 7, mWidth-10, 7);
      ofFill();
      ofRect(mZoomOffset*(mWidth-10)+5, mHeight - 7, (mWidth-10)/mZoomLevel, 7);
   }
   
   if (mHoveredCuePointIndex != -1 && mSample && mSample->LengthInSamples() > 0 && !mRecord)
   {
      float x = ofMap(mSampleCuePoints[mHoveredCuePointIndex].startSeconds, GetZoomStartSeconds(), GetZoomEndSeconds(), 0, sampleWidth);
      float xEnd = ofMap(mSampleCuePoints[mHoveredCuePointIndex].startSeconds + mSampleCuePoints[mHoveredCuePointIndex].lengthSeconds, GetZoomStartSeconds(), GetZoomEndSeconds(), 0, sampleWidth);
      if (xEnd - x > 45)
      {
         mPlayHoveredClipButton->SetPosition(x+5, 72);
         mGrabHoveredClipButton->SetPosition(x+28, 72);
         mPlayHoveredClipButton->SetShowing(true);
         mGrabHoveredClipButton->SetShowing(true);
         mPlayHoveredClipButton->Draw();
         mGrabHoveredClipButton->Draw();
      }
      else
      {
         mPlayHoveredClipButton->SetShowing(false);
         mGrabHoveredClipButton->SetShowing(false);
      }
   }
   else
   {
      mPlayHoveredClipButton->SetShowing(false);
      mGrabHoveredClipButton->SetShowing(false);
   }
}

float SamplePlayer::GetLengthInSeconds() const
{
   if (mSample != nullptr)
      return mSample->LengthInSamples() / (gSampleRate * mSample->GetSampleRateRatio());
   return 0;
}

int SamplePlayer::GetZoomStartSample() const
{
   if (mDoRecording)
      return 0;
   if (mSample == nullptr)
      return 0;
   return (int)ofClamp(mSample->LengthInSamples() * mZoomOffset, 0, mSample->LengthInSamples());
}

int SamplePlayer::GetZoomEndSample() const
{
   if (mDoRecording)
   {
      int numChunks = mRecordingLength / kRecordingChunkSize + 1;
      return (numChunks * kRecordingChunkSize);
   }
   if (mSample == nullptr)
      return 1;
   return (int)ofClamp(GetZoomStartSample() + mSample->LengthInSamples() / mZoomLevel, 1, mSample->LengthInSamples());
}

float SamplePlayer::GetZoomStartSeconds() const
{
   if (mDoRecording)
      return 0;
   if (mSample == nullptr)
      return 0;
   return GetZoomStartSample() / (gSampleRate * mSample->GetSampleRateRatio());
}

float SamplePlayer::GetZoomEndSeconds() const
{
   if (mDoRecording)
      return float(GetZoomEndSample()) / gSampleRate;
   if (mSample == nullptr)
      return 1;
   return GetZoomEndSample() / (gSampleRate * mSample->GetSampleRateRatio());
}

void SamplePlayer::oscMessageReceived(const OSCMessage& msg)
{
   if (msg.getAddressPattern().toString() == "/wheel/z")
   {
      bool grabbed = msg[0].getFloat32() > 0;
      if (grabbed)
      {
         mOscWheelPos = FLT_MAX;
         mOscWheelSpeed = 0;
         mOscWheelGrabbed = true;
      }
      else
      {
         mOscWheelGrabbed = false;
      }
   }
   else if (msg.getAddressPattern().toString() == "/wheel/x")
   {
      float pos = msg[0].getFloat32();
      if (mOscWheelPos == FLT_MAX)
      {
         mOscWheelPos = pos;
      }
      
      mOscWheelSpeed = (pos - mOscWheelPos) * 70;
      mOscWheelPos = pos;
   }
   else if (msg.getAddressPattern().toString() == "/Fader/x")
   {
      float pos = msg[0].getFloat32();
      mSpeed = ofLerp(mSpeedSlider->GetMin(), mSpeedSlider->GetMax(), pos);
   }
}

void SamplePlayer::oscBundleReceived(const OSCBundle& bundle)
{
   for (const OSCBundle::Element* element = bundle.begin(); element != bundle.end(); ++element)
   {
      if (element->isMessage())
         oscMessageReceived(element->getMessage());
      else if (element->isBundle())
         oscBundleReceived(element->getBundle());
   }
}

bool SamplePlayer::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (fabs(scrollX) > fabsf(scrollY))
      scrollY = 0;
   else
      scrollX = 0;

   //horizontal scroll
   mZoomOffset = ofClamp(mZoomOffset + scrollX*.005f, 0, 1);

   //zoom scroll
   float oldZoomLevel = mZoomLevel;
   mZoomLevel = ofClamp(mZoomLevel + scrollY*.2f, 1, 40);
   float zoomAmount = (mZoomLevel - oldZoomLevel) / oldZoomLevel; //find actual adjusted amount
   float zoomCenter = ofMap(x, 5, mWidth-10, 0, 1, true)/oldZoomLevel;
   mZoomOffset += zoomCenter * zoomAmount;
   if (mZoomLevel == 1)
      mZoomOffset = 0;

   return false;
}

void SamplePlayer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mLoopCheckbox)
   {
      if (mSample != nullptr)
         mSample->SetLooping(mLoop);
   }
   if (checkbox == mRecordCheckbox)
   {
      mPlay = false;
      if (mRecord)
      {
         if (!mRecordingAppendMode || mRecordingLength == 0)
         {
            mRecordingLength = 0;
            
            for (size_t i=mRecordChunks.size(); i<kMinRecordingChunks; ++i)
            {
               mRecordChunks.push_back(new ChannelBuffer(kRecordingChunkSize));
               mRecordChunks[i]->GetChannel(0); //set up buffer
            }
            
            for (size_t i=0; i<mRecordChunks.size(); ++i)
               mRecordChunks[i]->Clear();
            
            mRecordAsClipsCueIndex = 0;
            for (int i=0; i<(int)mSampleCuePoints.size(); ++i)
               SetCuePoint(i, 0, 0, 1);
         }
         
         mDoRecording = true;
      }
      else if (mRecordingLength > 0)
      {
         StopRecording();
      }
   }
   if (checkbox == mRecordAsClipsCheckbox)
      mRecordGate.SetEnabled(mRecordAsClips);
}

void SamplePlayer::StopRecording()
{
   if (mDoRecording)
   {
      mRecord = false;
      mDoRecording = false;
      
      Sample* sample = new Sample();
      sample->Create(mRecordingLength);
      ChannelBuffer* data = sample->Data();
      int channelCount = mRecordChunks[0]->NumActiveChannels();
      data->SetNumActiveChannels(channelCount);
      
      int numChunks = mRecordingLength / kRecordingChunkSize + 1;
      for (int i=0; i<numChunks; ++i)
      {
         int samplesLeftToRecord = mRecordingLength - i*kRecordingChunkSize;
         int samplesToCopy;
         if (samplesLeftToRecord > kRecordingChunkSize)
            samplesToCopy = kRecordingChunkSize;
         else
            samplesToCopy = samplesLeftToRecord;
         for (int ch=0; ch<channelCount; ++ch)
            BufferCopy(data->GetChannel(ch)+i*kRecordingChunkSize, mRecordChunks[i]->GetChannel(ch), samplesToCopy);
      }
      int recordedLength = mRecordingLength;
      UpdateSample(sample, true);
      mRecordingLength = recordedLength;
   }
}

void SamplePlayer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void SamplePlayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SamplePlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SamplePlayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("width", moduleInfo, mWidth);
   mModuleSaveData.LoadFloat("height", moduleInfo, mHeight);
   mModuleSaveData.LoadBool("show_youtube_process_output", moduleInfo, false);
   
   SetUpFromSaveData();
}

void SamplePlayer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void SamplePlayer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   Resize(mModuleSaveData.GetFloat("width"), mModuleSaveData.GetFloat("height"));
}

namespace
{
   const int kSaveStateRev = 2;
}

void SamplePlayer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   bool hasSample = (mSample != nullptr);
   out << hasSample;
   if (hasSample)
      mSample->SaveState(out);
   
   out << (int)mSampleCuePoints.size();
   for (size_t i=0; i<mSampleCuePoints.size(); ++i)
   {
      out << mSampleCuePoints[i].startSeconds;
      out << mSampleCuePoints[i].lengthSeconds;
      out << mSampleCuePoints[i].speed;
      out << mSampleCuePoints[i].stopOnNoteOff;
   }
}

void SamplePlayer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   bool hasSample;
   in >> hasSample;
   if (hasSample)
   {
      Sample* sample = new Sample();
      sample->LoadState(in);
      UpdateSample(sample, true);
   }
   
   if (rev >= 1)
   {
      int size;
      in >> size;
      mSampleCuePoints.resize(size);
      for (size_t i = 0; i < size; ++i)
      {
         in >> mSampleCuePoints[i].startSeconds;
         in >> mSampleCuePoints[i].lengthSeconds;
         in >> mSampleCuePoints[i].speed;
         if (rev >= 2)
            in >> mSampleCuePoints[i].stopOnNoteOff;
      }
   }
}

std::vector<IUIControl*> SamplePlayer::ControlsToIgnoreInSaveState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mDownloadYoutubeSearch);
   ignore.push_back(mLoadFileButton);
   ignore.push_back(mSaveFileButton);
   for (size_t i = 0; i < mSearchResultButtons.size(); ++i)
      ignore.push_back(mSearchResultButtons[i]);
   return ignore;
}

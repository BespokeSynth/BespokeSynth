/*
  ==============================================================================

    SampleCapturer.cpp
    Created: 12 Nov 2020 6:36:00pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SampleCapturer.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "Sample.h"

SampleCapturer::SampleCapturer()
   : IAudioProcessor(gBufferSize)
   , mCurrentSampleIndex(0)
   , mWantRecord(false)
   , mIsRecording(false)
   , mIsDragging(false)
{
}

void SampleCapturer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mWantRecordCheckbox, "record", &mWantRecord);
   BUTTON(mPlayButton, "play");
   BUTTON(mSaveButton, "save");
   BUTTON(mDeleteButton, "delete");
   ENDUIBLOCK0();
}

SampleCapturer::~SampleCapturer()
{
}

void SampleCapturer::Process(double time)
{
   PROFILER(SampleCapturer);

   if (!mEnabled)
      return;

   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();
   IAudioReceiver* target = GetTarget();

   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      gWorkChannelBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         BufferCopy(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      }

      for (int i = 0; i < bufferSize; ++i)
      {
         if (mWantRecord)
         {
            for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
            {
               if (mIsRecording || GetBuffer()->GetChannel(ch)[i] != 0)
               {
                  auto& sample = mSamples[mCurrentSampleIndex];
                  if (!mIsRecording)
                  {
                     mIsRecording = true;
                     sample.mBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
                     sample.mBuffer.Clear();
                  }

                  sample.mBuffer.GetChannel(ch)[sample.mRecordingLength] = GetBuffer()->GetChannel(ch)[i];
               }
            }

            if (mIsRecording)
            {
               ++mSamples[mCurrentSampleIndex].mRecordingLength;
               if (mSamples[mCurrentSampleIndex].mRecordingLength >= mSamples[mCurrentSampleIndex].mBuffer.BufferSize())
               {
                  mIsRecording = false;
                  mWantRecord = false;
                  mCurrentSampleIndex = (mCurrentSampleIndex + 1) % mSamples.size();
               }
            }
         }
         else
         {
            if (mIsRecording)
            {
               mIsRecording = false;
               mCurrentSampleIndex = (mCurrentSampleIndex + 1) % mSamples.size();
            }
         }
      }

      for (size_t sample = 0; sample < mSamples.size(); ++sample)
      {
         if (mSamples[sample].mPlaybackPos >= 0)
         {
            for (int i = 0; i < bufferSize; ++i)
            {
               for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
               {
                  out->GetChannel(ch)[i] += mSamples[sample].mBuffer.GetChannel(ch)[mSamples[sample].mPlaybackPos];
                  gWorkChannelBuffer.GetChannel(ch)[i] += mSamples[sample].mBuffer.GetChannel(ch)[mSamples[sample].mPlaybackPos];
               }
               ++mSamples[sample].mPlaybackPos;
               if (mSamples[sample].mPlaybackPos >= mSamples[sample].mRecordingLength)
               {
                  mSamples[sample].mPlaybackPos = -1;
                  break;
               }
            }
         }
      }

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

namespace
{
   const int kBufferStartY = 20;
   const int kBufferHeight = 60;
   const int kBufferWidth = 200;
   const int kBufferSpacing = 5;
}

void SampleCapturer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int y = kBufferStartY;
   for (size_t i = 0; i < mSamples.size(); ++i)
   {
      if (mSamples[i].mRecordingLength > 0)
      {
         ofPushMatrix();
         ofTranslate(5, y);
         DrawAudioBuffer(kBufferWidth, kBufferHeight, &mSamples[i].mBuffer, 0, mSamples[i].mBuffer.BufferSize(), mSamples[i].mPlaybackPos);
         ofPopMatrix();
      }

      if (mCurrentSampleIndex == i)
         ofSetColor(ofColor::white);
      else
         ofSetColor(IDrawableModule::GetColor(kModuleType_Audio));
      ofRect(5, y, kBufferWidth, kBufferHeight);

      if (i == mCurrentSampleIndex)
      {
         mPlayButton->SetPosition(10 + kBufferWidth, y);
         mSaveButton->PositionTo(mPlayButton, kAnchor_Below);
         mDeleteButton->PositionTo(mSaveButton, kAnchor_Below);
      }


      y += kBufferHeight + kBufferSpacing;
   }

   mWantRecordCheckbox->Draw();
   mPlayButton->Draw();
   mSaveButton->Draw();
   mDeleteButton->Draw();
}

void SampleCapturer::ButtonClicked(ClickButton* button)
{
   if (button == mPlayButton)
   {
      mSamples[mCurrentSampleIndex].mPlaybackPos = 0;
   }

   if (button == mSaveButton)
   {
      FileChooser chooser("Save sample as...", File(ofToDataPath(ofGetTimestampString("samples/%Y-%m-%d_%H-%M.wav"))), "*.wav", true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
      if (chooser.browseForFileToSave(true))
         Sample::WriteDataToFile(chooser.getResult().getFullPathName().toUTF8(), &mSamples[mCurrentSampleIndex].mBuffer, mSamples[mCurrentSampleIndex].mRecordingLength);
   }

   if (button == mDeleteButton)
   {
      mSamples[mCurrentSampleIndex].mBuffer.Clear();
      mSamples[mCurrentSampleIndex].mRecordingLength = 0;
   }
}

void SampleCapturer::GetModuleDimensions(float& w, float& h)
{
   w = kBufferWidth + 60;
   h = (int)mSamples.size() * (kBufferHeight + kBufferSpacing) + kBufferStartY;
}

void SampleCapturer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   for (int i = 0; i < (int)mSamples.size(); ++i)
   {
      if (y >= kBufferStartY + i * (kBufferHeight + kBufferSpacing) && y <= kBufferStartY + i * (kBufferHeight + kBufferSpacing) + kBufferHeight)
      {
         mCurrentSampleIndex = i;

         if (x < kBufferWidth + 10)
         {
            ChannelBuffer grab(mSamples[i].mRecordingLength);
            grab.SetNumActiveChannels(mSamples[i].mBuffer.NumActiveChannels());
            for (int ch = 0; ch < grab.NumActiveChannels(); ++ch)
               BufferCopy(grab.GetChannel(ch), mSamples[i].mBuffer.GetChannel(ch), mSamples[i].mRecordingLength);
            TheSynth->GrabSample(&grab, false, 1);
         }
      }
   }

   mIsDragging = true;
}

void SampleCapturer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mIsDragging = false;
}

bool SampleCapturer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (mIsDragging)
   {

   }

   return false;
}

void SampleCapturer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SampleCapturer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void SampleCapturer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;

   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i].mBuffer.Save(out, mSamples[i].mBuffer.BufferSize());
}

void SampleCapturer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);

   int readLength;
   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i].mBuffer.Load(in, readLength, ChannelBuffer::LoadMode::kSetBufferSize);
}

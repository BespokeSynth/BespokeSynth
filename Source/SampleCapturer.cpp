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
#include "Checkbox.h"

#include "juce_gui_basics/juce_gui_basics.h"

SampleCapturer::SampleCapturer()
: IAudioProcessor(gBufferSize)
{
}

void SampleCapturer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mWantRecordCheckbox, "record", &mWantRecord);
   BUTTON_STYLE(mPlayButton, "play", ButtonDisplayStyle::kPlay);
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

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();

      return;
   }

   int bufferSize = GetBuffer()->BufferSize();

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
         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Audio));
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

void SampleCapturer::ButtonClicked(ClickButton* button, double time)
{
   using namespace juce;
   if (button == mPlayButton)
   {
      mSamples[mCurrentSampleIndex].mPlaybackPos = 0;
   }

   if (button == mSaveButton)
   {
      FileChooser chooser("Save sample as...", File(ofToSamplePath(ofGetTimestampString("%Y-%m-%d_%H-%M-%S.wav"))), "*.wav", true, false, TheSynth->GetFileChooserParent());
      if (chooser.browseForFileToSave(true))
         Sample::WriteDataToFile(chooser.getResult().getFullPathName().toStdString(), &mSamples[mCurrentSampleIndex].mBuffer, mSamples[mCurrentSampleIndex].mRecordingLength);
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

void SampleCapturer::OnClicked(float x, float y, bool right)
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
            TheSynth->GrabSample(&grab, "captured", false);
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

void SampleCapturer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i].mBuffer.Save(out, mSamples[i].mBuffer.BufferSize());
}

void SampleCapturer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int readLength;
   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i].mBuffer.Load(in, readLength, ChannelBuffer::LoadMode::kSetBufferSize);
}

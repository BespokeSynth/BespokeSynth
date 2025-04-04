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
//  Sampler.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#include "Sampler.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Sample.h"
#include "Profiler.h"
#include "Scale.h"
#include "UIControlMacros.h"

Sampler::Sampler()
: IAudioProcessor(gBufferSize)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mVoiceParams.mVol = .5f;
   mVoiceParams.mAdsr.Set(10, 0, 1, 10);
   mVoiceParams.mSample = &mSample;
   mVoiceParams.mSamplePitch = 48;

   mPolyMgr.Init(kVoiceType_Sampler, &mVoiceParams);

   //mWriteBuffer.SetNumActiveChannels(2);
}

void Sampler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mADSRDisplay = new ADSRDisplay(this, "env", 3, 3, 80, 50, &mVoiceParams.mAdsr);

   UIBLOCK(3, 56);
   FLOATSLIDER(mVolSlider, "vol", &mVoiceParams.mVol, 0, 1);
   TEXTENTRY_NUM(mSamplePitchEntry, "sample pitch", 6, &mVoiceParams.mSamplePitch, 0, 127);
   BUTTON(mDetectPitchButton, "detect pitch");
   CHECKBOX(mRecordCheckbox, "rec", &mRecording);
   FLOATSLIDER(mThreshSlider, "thresh", &mThresh, 0, 1);
   CHECKBOX(mPassthroughCheckbox, "passthrough", &mPassthrough);
   UIBLOCK_NEWCOLUMN();
   UIBLOCK_PUSHSLIDERWIDTH(200);
   INTSLIDER(mStartSlider, "start", &mVoiceParams.mStartSample, 0, 1000);
   INTSLIDER(mStopSlider, "stop", &mVoiceParams.mStopSample, -1, 1000);
   CHECKBOX(mSustainLoopCheckbox, "sustain loop", &mSustainLoop);
   INTSLIDER(mSustainLoopStartSlider, "sustain start", &mVoiceParams.mSustainLoopStart, -1, 1000);
   INTSLIDER(mSustainLoopEndSlider, "sustain end", &mVoiceParams.mSustainLoopEnd, -1, 1000);
   ENDUIBLOCK(mWidth, mHeight);
}

Sampler::~Sampler()
{
}

void Sampler::Poll()
{
}

void Sampler::Process(double time)
{
   PROFILER(Sampler);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   mNoteInputBuffer.Process(time);

   ComputeSliders(0);
   SyncInputBuffer();
   int numChannels = 2;
   SyncOutputBuffer(numChannels);
   mWriteBuffer.SetNumActiveChannels(numChannels);

   int bufferSize = GetBuffer()->BufferSize();

   mWriteBuffer.Clear();

   if (mRecording)
   {
      for (int i = 0; i < gBufferSize; ++i)
      {
         //if we've already started recording, or if it's a new recording and there's sound
         if (mRecordPos > 0 || fabsf(GetBuffer()->GetChannel(0)[i]) > mThresh)
         {
            mSample.Data()->GetChannel(0)[mRecordPos] = GetBuffer()->GetChannel(0)[i];
            if (mPassthrough)
            {
               for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
                  mWriteBuffer.GetChannel(ch)[i] += mSample.Data()->GetChannel(0)[mRecordPos];
            }
            ++mRecordPos;
         }

         if (mRecordPos >= mSample.LengthInSamples())
         {
            StopRecording();
            break;
         }
      }
   }

   mSample.LockDataMutex(true);
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   mSample.LockDataMutex(false);

   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }

   GetBuffer()->Reset();
}

void Sampler::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity > 0)
   {
      mMostRecentVoiceIdx = mPolyMgr.Start(note.time, note.pitch, note.velocity / 127.0f, note.voiceIdx, note.modulation);
      mVoiceParams.mAdsr.Start(note.time, 1); //for visualization
   }
   else
   {
      mPolyMgr.Stop(note.time, note.pitch, note.voiceIdx);
      mVoiceParams.mAdsr.Stop(note.time); //for visualization
   }

   if (mDrawDebug)
   {
      mDebugLines[mDebugLinesPos].text = "PlayNote(" + ofToString(note.time / 1000) + ", " + ofToString(note.pitch) + ", " + ofToString(note.velocity) + ", " + ofToString(note.voiceIdx) + ")";
      if (note.velocity > 0)
         mDebugLines[mDebugLinesPos].color = ofColor::lime;
      else
         mDebugLines[mDebugLinesPos].color = ofColor::red;
      ofLog() << mDebugLines[mDebugLinesPos].text;
      mDebugLinesPos = (mDebugLinesPos + 1) % (int)mDebugLines.size();
   }
}

void Sampler::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void Sampler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSustainLoopStartSlider->SetShowing(mSustainLoop);
   mSustainLoopEndSlider->SetShowing(mSustainLoop);

   mVolSlider->Draw();
   mADSRDisplay->Draw();
   mRecordCheckbox->Draw();
   mThreshSlider->Draw();
   mDetectPitchButton->Draw();
   mPassthroughCheckbox->Draw();
   mSamplePitchEntry->Draw();
   mStartSlider->Draw();
   mStopSlider->Draw();
   mSustainLoopCheckbox->Draw();
   mSustainLoopStartSlider->Draw();
   mSustainLoopEndSlider->Draw();

   ofPushMatrix();
   ofTranslate(106, 3);
   float pos = 0;
   if (mMostRecentVoiceIdx != -1)
   {
      auto& voiceInfo = mPolyMgr.GetVoiceInfo(mMostRecentVoiceIdx);
      SampleVoice* voice = dynamic_cast<SampleVoice*>(voiceInfo.mVoice);
      pos = voice->GetSamplePosition();
   }
   DrawAudioBuffer(200, 50, mSample.Data(), 0, mSample.LengthInSamples(), pos);
   DrawTextNormal(std::string(mSample.Name()), 5, 10);
   ofPushStyle();
   ofNoFill();
   ofSetColor(255, 0, 0);
   if (mRecording && mRecordPos > 0)
      ofRect(0, 0, 100, 50);
   ofPopStyle();
   ofPopMatrix();
}

void Sampler::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(mWidth + 3, 0);
      float y = mHeight + 15;
      for (size_t i = 0; i < mDebugLines.size(); ++i)
      {
         const DebugLine& line = mDebugLines[(mDebugLinesPos + i) % mDebugLines.size()];
         ofSetColor(line.color);
         DrawTextNormal(line.text, 0, y);
         y += 15;
      }
   }
}

void Sampler::StopRecording()
{
   mRecording = false;
   mSample.SetStopPoint(mRecordPos);
}

float Sampler::DetectSamplePitch()
{
   float pitch = mPitchDetector.DetectPitch(mSample.Data()->GetChannel(0), mSample.LengthInSamples());
   ofLog() << "Detected pitch: " << pitch;
   return pitch;
}

void Sampler::UpdateForNewSample()
{
   mStartSlider->SetExtents(0, mSample.LengthInSamples());
   mStopSlider->SetExtents(0, mSample.LengthInSamples());
   mSustainLoopStartSlider->SetExtents(0, mSample.LengthInSamples());
   mSustainLoopEndSlider->SetExtents(0, mSample.LengthInSamples());
   mVoiceParams.mStopSample = mSample.LengthInSamples();
   mSustainLoop = false;
   mVoiceParams.mSustainLoopStart = -1;
   mVoiceParams.mSustainLoopEnd = -1;
}

void Sampler::FilesDropped(std::vector<std::string> files, int x, int y)
{
   mSample.LockDataMutex(true);
   mSample.Read(files[0].c_str());
   mSample.LockDataMutex(false);
   UpdateForNewSample();
}

void Sampler::SampleDropped(int x, int y, Sample* sample)
{
   mSample.LockDataMutex(true);
   mSample.CopyFrom(sample);
   mSample.LockDataMutex(false);
   UpdateForNewSample();
}

void Sampler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Sampler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}


void Sampler::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void Sampler::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Sampler::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void Sampler::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mPolyMgr.KillAll();

   if (checkbox == mRecordCheckbox)
   {
      if (mRecording)
      {
         mRecordPos = 0;
         mSample.LockDataMutex(true);
         mSample.Create(3.0f * gSampleRate);
         mSample.SetName("recorded");
         mSample.LockDataMutex(false);
         UpdateForNewSample();
      }
      else
      {
         StopRecording();
      }
   }

   if (checkbox == mSustainLoopCheckbox)
   {
      if (!mSustainLoop)
      {
         mVoiceParams.mSustainLoopStart = -1;
         mVoiceParams.mSustainLoopEnd = -1;
      }
   }
}

void Sampler::ButtonClicked(ClickButton* button, double time)
{
   if (button == mDetectPitchButton)
      mVoiceParams.mSamplePitch = DetectSamplePitch();
}

void Sampler::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mSample.SaveState(out);
}

void Sampler::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 3)
   {
      mSample.LoadState(in);
   }
   else
   {
      int length = 2 * 48000;
      if (rev < 2)
         length = 2 * gSampleRate;

      mSample.Create(length);
      in.Read(mSample.Data()->GetChannel(0), length);

      int sampleLength;
      if (rev >= 1)
         in >> sampleLength;
   }
}

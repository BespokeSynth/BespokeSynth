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
#include "EnvOscillator.h"
#include "Scale.h"

Sampler::Sampler()
: IAudioProcessor(gBufferSize)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mVolSlider(nullptr)
, mADSRDisplay(nullptr)
, mRecordPos(0)
, mRecording(false)
, mRecordCheckbox(nullptr)
, mThresh(.2f)
, mThreshSlider(nullptr)
, mPitchCorrect(false)
, mPitchCorrectCheckbox(nullptr)
, mWantDetectPitch(false)
, mPassthrough(false)
, mPassthroughCheckbox(nullptr)
, mWriteBuffer(gBufferSize)
{
   mSampleData = new float[MAX_SAMPLER_LENGTH];   //store up to 2 seconds
   Clear(mSampleData, MAX_SAMPLER_LENGTH);
   
   mVoiceParams.mVol = .5f;
   mVoiceParams.mAdsr.Set(10,0,1,10);
   mVoiceParams.mSampleData = mSampleData;
   mVoiceParams.mSampleLength = 0;
   mVoiceParams.mDetectedFreq = -1;
   mVoiceParams.mLoop = false;
   
   mPolyMgr.Init(kVoiceType_Sampler, &mVoiceParams);
   
   //mWriteBuffer.SetNumActiveChannels(2);
}

void Sampler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",5,73,80,15,&mVoiceParams.mVol,0,1);
   mADSRDisplay = new ADSRDisplay(this,"env",5,15,80,40,&mVoiceParams.mAdsr);
   mRecordCheckbox = new Checkbox(this,"rec",5,57,&mRecording);
   mThreshSlider = new FloatSlider(this,"thresh",90,73,80,15,&mThresh,0,1);
   mPitchCorrectCheckbox = new Checkbox(this,"pitch",60,57,&mPitchCorrect);
   mPassthroughCheckbox = new Checkbox(this,"passthrough",70,0,&mPassthrough);
   
   mADSRDisplay->SetVol(mVoiceParams.mVol);
}

Sampler::~Sampler()
{
   delete[] mSampleData;
}

void Sampler::Poll()
{
   if (mWantDetectPitch)
   {
      mVoiceParams.mDetectedFreq = DetectSampleFrequency();
      mWantDetectPitch = false;
   }
}

void Sampler::Process(double time)
{
   PROFILER(Sampler);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;
   
   mNoteInputBuffer.Process(time);
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   
   mWriteBuffer.Clear();
   
   if (mRecording)
   {
      for (int i=0; i<gBufferSize; ++i)
      {
         //if we've already started recording, or if it's a new recording and there's sound
         if (mRecordPos > 0 || fabsf(GetBuffer()->GetChannel(0)[i]) > mThresh )
         {
            mSampleData[mRecordPos] = GetBuffer()->GetChannel(0)[i];
            if (mPassthrough)
            {
               for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
                  mWriteBuffer.GetChannel(ch)[i] += mSampleData[mRecordPos];
            }
            ++mRecordPos;
         }
         
         if (mRecordPos >= MAX_SAMPLER_LENGTH)
         {
            StopRecording();
            break;
         }
      }
   }
   
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   
   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
   
   GetBuffer()->Reset();
}

void Sampler::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
      mVoiceParams.mAdsr.Start(time,1);         //for visualization
   }
   else
   {
      mPolyMgr.Stop(time, pitch);
      mVoiceParams.mAdsr.Stop(time);         //for visualization
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
   
   mVolSlider->Draw();
   mADSRDisplay->Draw();
   mRecordCheckbox->Draw();
   mThreshSlider->Draw();
   mPitchCorrectCheckbox->Draw();
   mPassthroughCheckbox->Draw();
   
   ofPushMatrix();
   ofTranslate(100,15);
   DrawAudioBuffer(100, 50, mSampleData, 0, mVoiceParams.mSampleLength, -1);
   ofPushStyle();
   ofNoFill();
   ofSetColor(255,0,0);
   if (mRecording && mRecordPos > 0)
      ofRect(0,0,100,50);
   ofPopStyle();
   ofPopMatrix();
}

void Sampler::StopRecording()
{
   mRecording = false;
   mVoiceParams.mSampleLength = mRecordPos;
   if (mPitchCorrect)
      mWantDetectPitch = true;
}

float Sampler::DetectSampleFrequency()
{
   /*EnvOscillator osc(kOsc_Sin);
   osc.Start(0,1);
   float time = 0;
   float phase = 0;
   float phaseInc = GetPhaseInc(440);
   for (int i=0; i<MAX_SAMPLER_LENGTH; ++i)
   {
      phase += phaseInc;
      while (phase > FTWO_PI) { phase -= FTWO_PI; }
      
      mSampleData[i] = osc.Audio(time, phase);
      
      time += gInvSampleRateMs;
   }*/
   
   float pitch = mPitchDetector.DetectPitch(mSampleData, MAX_SAMPLER_LENGTH);
   float freq = TheScale->PitchToFreq(pitch);
   ofLog() << "Detected frequency: " << freq;
   return freq;
}

void Sampler::GetModuleDimensions(float& width, float& height)
{
   width = 210;
   height = 90;
}

void Sampler::FilesDropped(std::vector<std::string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x,y,&sample);
}

void Sampler::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   //TODO(Ryan) multichannel
   const float* data = sample->Data()->GetChannel(0);
   int numSamples = sample->LengthInSamples();
   
   if (numSamples <= 0)
      return;
   
   mVoiceParams.mSampleLength = MIN(MAX_SAMPLER_LENGTH, numSamples);
   Clear(mSampleData, MAX_SAMPLER_LENGTH);
   
   for (int i=0; i<mVoiceParams.mSampleLength; ++i)
      mSampleData[i] = data[i];
}

void Sampler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("loop", moduleInfo, false);
   
   SetUpFromSaveData();
}

void Sampler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mVoiceParams.mLoop = mModuleSaveData.GetBool("loop");
}


void Sampler::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void Sampler::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mVolSlider)
      mADSRDisplay->SetVol(mVoiceParams.mVol);
}

void Sampler::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   
}

void Sampler::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mRecordCheckbox)
   {
      if (mRecording)
      {
         mRecordPos = 0;
         mVoiceParams.mSampleLength = 0;
         Clear(mSampleData, MAX_SAMPLER_LENGTH);
      }
      else
      {
         StopRecording();
      }
   }
   if (checkbox == mPitchCorrectCheckbox)
   {
      if (mPitchCorrect)
         mVoiceParams.mDetectedFreq = DetectSampleFrequency();
      else
         mVoiceParams.mDetectedFreq = -1;
   }
}

namespace
{
   const int kSaveStateRev = 1;
}

void Sampler::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out.Write(mSampleData, MAX_SAMPLER_LENGTH);
   out << mVoiceParams.mSampleLength;
}

void Sampler::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in.Read(mSampleData, MAX_SAMPLER_LENGTH);
   
   if (rev >= 1)
      in >> mVoiceParams.mSampleLength;
   
   if (mPitchCorrect)
      mWantDetectPitch = true;
}


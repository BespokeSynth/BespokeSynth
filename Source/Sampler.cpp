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
: mVolSlider(NULL)
, mADSRDisplay(NULL)
, mRecordPos(0)
, mRecording(false)
, mRecordCheckbox(NULL)
, mThresh(.2f)
, mThreshSlider(NULL)
, mPitchCorrect(false)
, mPitchCorrectCheckbox(NULL)
, mWantDetectPitch(false)
, mPassthrough(false)
, mPassthroughCheckbox(NULL)
, mPolyMgr(this)
{
   mSampleData = new float[MAX_SAMPLER_LENGTH];   //store up to 2 seconds
   Clear(mSampleData, MAX_SAMPLER_LENGTH);
   
   mVoiceParams.mVol = .05f;
   mVoiceParams.mAdsr.Set(10,0,1,10);
   mVoiceParams.mSampleData = mSampleData;
   mVoiceParams.mSampleLength = 0;
   mVoiceParams.mDetectedFreq = -1;
   
   mPolyMgr.Init(kVoiceType_Sampler, &mVoiceParams);
   
   mWriteBuffer = new float[gBufferSize];
   mRecordBuffer = new float[gBufferSize];
}

void Sampler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",5,73,80,15,&mVoiceParams.mVol,0,1);
   mADSRDisplay = new ADSRDisplay(this,"adsr",5,15,80,40,&mVoiceParams.mAdsr);
   mRecordCheckbox = new Checkbox(this,"rec",5,57,&mRecording);
   mThreshSlider = new FloatSlider(this,"thresh",90,73,80,15,&mThresh,0,1);
   mPitchCorrectCheckbox = new Checkbox(this,"pitch",60,57,&mPitchCorrect);
   mPassthroughCheckbox = new Checkbox(this,"passthrough",70,0,&mPassthrough);
   
   mADSRDisplay->SetVol(mVoiceParams.mVol);
}

Sampler::~Sampler()
{
   delete[] mSampleData;
   delete[] mWriteBuffer;
   delete[] mRecordBuffer;
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
   Profiler profiler("Sampler");

   if (!mEnabled || GetTarget() == NULL)
      return;
   
   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   
   if (mRecording)
   {
      for (int i=0; i<gBufferSize; ++i)
      {
         //if we've already started recording, or if it's a new recording and there's sound
         if (mRecordPos > 0 || fabsf(mRecordBuffer[i]) > mThresh )
         {
            mSampleData[mRecordPos] = mRecordBuffer[i];
            if (mPassthrough)
               mWriteBuffer[i] += mSampleData[mRecordPos];
            ++mRecordPos;
         }
         
         if (mRecordPos >= MAX_SAMPLER_LENGTH)
         {
            StopRecording();
            break;
         }
      }
   }
   
   mPolyMgr.Process(time, mWriteBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize);
   
   Add(out, mWriteBuffer, bufferSize);
   
   Clear(mRecordBuffer, gBufferSize);
}

void Sampler::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, pitchBend, modWheel, pressure);
   else
      mPolyMgr.Stop(time, pitch);
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

void Sampler::GetModuleDimensions(int& width, int& height)
{
   width = 210;
   height = 90;
}

void Sampler::FilesDropped(vector<string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x,y,&sample);
}

void Sampler::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   const float* data = sample->Data();
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
   mModuleSaveData.LoadFloat("vol", moduleInfo, .6f, mVolSlider);
   
   SetUpFromSaveData();
}

void Sampler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetVol(mModuleSaveData.GetFloat("vol"));
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
   const int kSaveStateRev = 0;
}

void Sampler::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out.Write(mSampleData, MAX_SAMPLER_LENGTH);
}

void Sampler::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in.Read(mSampleData, MAX_SAMPLER_LENGTH);
}


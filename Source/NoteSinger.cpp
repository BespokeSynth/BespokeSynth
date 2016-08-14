//
//  NoteSinger.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/23/13.
//
//

#include "NoteSinger.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

NoteSinger::NoteSinger()
: mPitch(0)
, mOctave(0)
, mOctaveSlider(NULL)
, mNumBuckets(28)
{
   SetEnabled(false);
   
   TheTransport->AddAudioPoller(this);
   TheScale->AddListener(this);

   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   
   mWorkBuffer = new float[mInputBufferSize];
   Clear(mWorkBuffer, mInputBufferSize);
   
   OnScaleChanged();
}

void NoteSinger::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mOctaveSlider = new IntSlider(this,"oct",35,32,60,15,&mOctave,-2,2);
}

NoteSinger::~NoteSinger()
{
   TheTransport->RemoveAudioPoller(this);
   TheScale->RemoveListener(this);
   delete[] mInputBuffer;
}

float* NoteSinger::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void NoteSinger::OnTransportAdvanced(float amount)
{
   Profiler profiler("NoteSinger");
   
   if (!mEnabled)
      return;

   ComputeSliders(0);
   
   int pitch = -1;
   
   int bestBucket = -1;
   float bestPeak = -1;
   for (int i=0; i<mNumBuckets; ++i)
   {
      memcpy(mWorkBuffer,mInputBuffer, mInputBufferSize*sizeof(float));
      mBands[i].Filter(mWorkBuffer, mInputBufferSize);
      mPeaks[i].Process(mWorkBuffer, mInputBufferSize);
      
      float peak = mPeaks[i].GetPeak();
      if (peak > bestPeak)
      {
         int numPitchesInScale = TheScale->NumPitchesInScale();
         if (bestBucket != -1 &&
             peak < 2*bestPeak &&
             i % numPitchesInScale == bestBucket % numPitchesInScale)
            continue; //don't let a harmonic beat a fundamental
         
         bestBucket = i;
         bestPeak = peak;
      }
   }
   assert(bestBucket != -1);
   
   pitch = GetPitchForBucket(bestBucket) + mOctave * 12;
   
   if (pitch != mPitch && bestPeak > .01f)
   {
      PlayNoteOutput(gTime, pitch, 80, -1);
      PlayNoteOutput(gTime, mPitch, 0, -1);
      mPitch = pitch;
   }

   Clear(mInputBuffer, mInputBufferSize);
}

string NoteSinger::GetTitleLabel()
{
   if (Minimized())
      return "notesinger";
   return "notesinger "+ofToString(mPitch,2);
}

void NoteSinger::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mOctaveSlider->Draw();

   for (int i=0; i<mNumBuckets; ++i)
   {
      float x = ofMap(i,0,mNumBuckets,0,100);
      ofSetColor(255,0,255);
      ofLine(x,50,x,50-ofMap(MIN(mPeaks[i].GetPeak(),1),0,1,0,50));
   }
}

void NoteSinger::OnScaleChanged()
{
   mNumBuckets = MIN(TheScale->NumPitchesInScale() * 4, NOTESINGER_MAX_BUCKETS);
   
   for (int i=0; i<mNumBuckets; ++i)
   {
      int pitch = GetPitchForBucket(i);
      float f = TheScale->PitchToFreq(pitch);
      
      mBands[i].SetFilterType(kFilterType_Bandpass);
      mBands[i].SetFilterParams(f, 40+mNumBuckets*2-i*2);
      
      mPeaks[i].SetDecayTime(.05f);
   }
}

int NoteSinger::GetPitchForBucket(int bucket)
{
   return TheScale->GetPitchFromTone(bucket+TheScale->NumPitchesInScale()*2);
}

void NoteSinger::ButtonClicked(ClickButton* button)
{
}

void NoteSinger::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      PlayNoteOutput(gTime, mPitch, 0, -1);
      mPitch = -1;
   }
}

void NoteSinger::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteSinger::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


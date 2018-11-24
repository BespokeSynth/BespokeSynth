//
//  MultibandCompressor.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/27/14.
//
//

#include "MultibandCompressor.h"
#include "ModularSynth.h"
#include "Profiler.h"

MultibandCompressor::MultibandCompressor()
: IAudioProcessor(gBufferSize)
, mDryWet(1)
, mDryWetSlider(nullptr)
, mNumBands(4)
, mNumBandsSlider(nullptr)
, mFreqMin(150)
, mFMinSlider(nullptr)
, mFreqMax(7500)
, mFMaxSlider(nullptr)
, mRingTime(.01f)
, mRingTimeSlider(nullptr)
, mMaxBand(.3f)
, mMaxBandSlider(nullptr)

{
   mWorkBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mWorkBuffer, GetBuffer()->BufferSize());
   
   mOutBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mOutBuffer, GetBuffer()->BufferSize());
   
   CalcFilters();
}

void MultibandCompressor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDryWetSlider = new FloatSlider(this,"dry/wet", 5, 83, 100, 15, &mDryWet, 0, 1);
   mNumBandsSlider = new IntSlider(this,"bands", 110, 29, 100, 15, &mNumBands, 1, COMPRESSOR_MAX_BANDS);
   mFMinSlider = new FloatSlider(this,"fmin", 110, 47, 100, 15, &mFreqMin, 70, 400);
   mFMaxSlider = new FloatSlider(this,"fmax", 110, 65, 100, 15, &mFreqMax, 300, gSampleRate/2-1);
   mRingTimeSlider = new FloatSlider(this, "ring", 110, 101, 100, 15, &mRingTime, .0001f, .1f, 4);
   mMaxBandSlider = new FloatSlider(this,"max band", 5, 101, 100, 15, &mMaxBand, 0.001f, 1);
}

MultibandCompressor::~MultibandCompressor()
{
   delete[] mOutBuffer;
   delete[] mWorkBuffer;
}

void MultibandCompressor::Process(double time)
{
   PROFILER(multiband);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      Clear(mOutBuffer, bufferSize);
      
      for (int i=0; i<bufferSize; ++i)
      {
         float lower;
         float highLeftover = GetBuffer()->GetChannel(0)[i];
         for (int j=0; j<mNumBands; ++j)
         {
            mFilters[j].ProcessSample(highLeftover, lower, highLeftover);
            mPeaks[j].Process(&lower, 1);
            float compress = ofClamp(1/mPeaks[i].GetPeak(), 0, 10);
            mOutBuffer[i] += lower * compress;
         }
         mOutBuffer[i] += highLeftover;
      }
      
      /*for (int i=0; i<mNumBands; ++i)
      {
         //get carrier band
         BufferCopy(mWorkBuffer, mInputBuffer, bufferSize);
         
         mFilters[i].ProcessSample(const double &sample, double &lowOut, double &highOut)(mWorkBuffer, bufferSize);
         
         //calculate modulator band level
         mPeaks[i].Process(mWorkBuffer, bufferSize);
         
         //multiply carrier band by modulator band level
         if (mPeaks[i].GetPeak() > 0)
         {
            float compress = ofClamp(1/mPeaks[i].GetPeak(), 0, 10);
            Mult(mWorkBuffer, compress, bufferSize);
         }
         
         //accumulate output band into total output
         Add(mOutBuffer, mWorkBuffer, bufferSize);
      }*/
      
      Mult(GetBuffer()->GetChannel(0), (1-mDryWet), bufferSize);
      Mult(mOutBuffer, mDryWet, bufferSize);
      
      Add(GetTarget()->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);
      Add(GetTarget()->GetBuffer()->GetChannel(0), mOutBuffer, bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize, 0);
   
   GetBuffer()->Reset();
}

void MultibandCompressor::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mDryWetSlider->Draw();
   mFMinSlider->Draw();
   mFMaxSlider->Draw();
   mNumBandsSlider->Draw();
   mRingTimeSlider->Draw();
   mMaxBandSlider->Draw();
   
   ofPushStyle();
   ofFill();
   ofSetColor(0,255,0);
   const float width = 25;
   for (int i=0; i<mNumBands; ++i)
   {
      ofRect(i*(width+3),-mPeaks[i].GetPeak()*200,width,mPeaks[i].GetPeak()*200);
   }
   ofPopStyle();
}

void MultibandCompressor::CalcFilters()
{
   for (int i=0; i<mNumBands; ++i)
   {
      float a = float(i)/mNumBands;
      float f = mFreqMin * powf(mFreqMax/mFreqMin, a);
      
      mFilters[i].SetCrossoverFreq(f);
   }
}

void MultibandCompressor::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumBandsSlider)
   {
      CalcFilters();
   }
}

void MultibandCompressor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mFMinSlider || slider == mFMaxSlider)
   {
      CalcFilters();
   }
   if (slider == mRingTimeSlider)
   {
      for (int i=0; i<COMPRESSOR_MAX_BANDS; ++i)
         mPeaks[i].SetDecayTime(mRingTime);
   }
   if (slider == mMaxBandSlider)
   {
      for (int i=0; i<COMPRESSOR_MAX_BANDS; ++i)
         mPeaks[i].SetLimit(mMaxBand);
   }
}

void MultibandCompressor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void MultibandCompressor::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}



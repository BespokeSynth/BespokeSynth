//
//  OneShotLauncher.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 10/16/14.
//
//

#include "OneShotLauncher.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SampleBank.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"

OneShotLauncher::OneShotLauncher()
: mVolume(1)
, mVolumeSlider(nullptr)
, mSampleIndex(-1)
, mSampleList(nullptr)
, mBank(nullptr)
, mNumActive(0)
, mAddBeatButton(nullptr)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);
}

void OneShotLauncher::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",5,20,110,15,&mVolume,0,2);
   mSampleList = new DropdownList(this,"samples",25,40,&mSampleIndex);
   mAddBeatButton = new ClickButton(this,"+",5,40);
}

OneShotLauncher::~OneShotLauncher()
{
   delete[] mWriteBuffer;
   TheTransport->RemoveListener(this);
   
   /*for (int i=0; i<MAX_OneShotLauncher; ++i)
   {
      delete mBeatData[i].mRemoveButton;
      delete mBeatData[i].mVolumeSlider;
   }*/
}

void OneShotLauncher::Process(double time)
{
   /*PROFILER(OneShotLauncher);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   ComputeSliders(0);
   
    int bufferSize = GetTarget()->GetBuffer()->BufferSize();
    float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   for (int idx = 0; idx < mNumActive; ++idx)
   {
      Sample* beat = mBeatData[idx].mBeat;
      float volSq = mVolume * mVolume * mBeatData[idx].mVolume * mBeatData[idx].mVolume * .25f;
      
      float speed = beat->LengthInSamples() * gInvSampleRateMs / TheTransport->MsPerBar() / mBeatData[idx].mNumBars;
      RecalcPos(idx);
      beat->SetRate(speed);
      
      if (beat->ConsumeData(time, mWriteBuffer, bufferSize, true))
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = mWriteBuffer[i] * volSq;
            out[i] += sample;
            GetVizBuffer()->Write(sample, 0);
         }
      }
   }*/
}

void OneShotLauncher::SetSampleBank(SampleBank* bank)
{
   mBank = bank;
   
   UpdateSampleList();
}

void OneShotLauncher::UpdateSampleList()
{
   if (mBank == nullptr)
      return;
   
   mSampleList->Clear();
   vector<SampleInfo> samples = mBank->GetSamples();
   for (int i=0; i<samples.size(); ++i)
   {
      mSampleList->AddLabel(samples[i].mSample->Name(), i);
   }
}

void OneShotLauncher::DropdownClicked(DropdownList* list)
{
   if (list == mSampleList)
   {
      UpdateSampleList();
   }
}

void OneShotLauncher::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mSampleList)
   {
   }
}

void OneShotLauncher::OnTimeEvent(double time)
{
}

void OneShotLauncher::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mSampleList->Draw();
   mAddBeatButton->Draw();
   
   /*for (int i=0; i < mNumActive; ++i)
   {
      Sample* beat = mBeatData[i].mBeat;
      DrawTextNormal(beat->Name(), 85, GetRowY(i)+12);
      mBeatData[i].mRemoveButton->Draw();
      mBeatData[i].mVolumeSlider->Draw();
   }*/
}

int OneShotLauncher::GetRowY(int idx)
{
   return 61+idx*15;
}

void OneShotLauncher::ButtonClicked(ClickButton* button)
{
   /*if (button == mAddBeatButton)
   {
      AddBeat();
   }
   for (int i=0; i<MAX_OneShotLauncher; ++i)
   {
      if (button == mBeatData[i].mRemoveButton)
      {
         for (int j=i; j<mNumActive; ++j)
         {
            mBeatData[j].mBeat = mBeatData[j+1].mBeat;
            mBeatData[j].mVolume = mBeatData[j+1].mVolume;
            mBeatData[j].mNumBars = mBeatData[j+1].mNumBars;
         }
         --mNumActive;
      }
   }*/
}

void OneShotLauncher::CheckboxUpdated(Checkbox *checkbox)
{
}

void OneShotLauncher::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 125;
}

void OneShotLauncher::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void OneShotLauncher::RecalcPos(int idx)
{
   /*int numBars = mBeatData[idx].mNumBars;
   Sample* beat = mBeatData[idx].mBeat;
   
   float measurePos = TheTransport->GetMeasure() % numBars + TheTransport->GetMeasurePos();
   int pos = ofMap(measurePos/numBars, 0, 1, 0, beat->LengthInSamples(), true);
   beat->SetPlayPosition(pos);*/
}

void OneShotLauncher::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void OneShotLauncher::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("samplebank", moduleInfo, "", FillDropdown<SampleBank*>);
   
   SetUpFromSaveData();
}

void OneShotLauncher::SetUpFromSaveData()
{
   /*SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetSampleBank(dynamic_cast<SampleBank*>(TheSynth->FindModule(mModuleSaveData.GetString("samplebank"),false)));
   
   for (int i=0; i<mOneShots.size(); ++i)
   {
      mBeatData[i].mRemoveButton = new ClickButton(this,ofToString(i+1),5,GetRowY(i));
      mBeatData[i].mVolumeSlider = new FloatSlider(this,"vol",25,GetRowY(i),53,15,&mBeatData[i].mVolume,0,2);
   }*/
}


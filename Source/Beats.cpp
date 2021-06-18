//
//  Beats.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/2/14.
//
//

#include "Beats.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SampleBank.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

Beats::Beats()
: mRows(4)
, mWriteBuffer(gBufferSize)
{
   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);

   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      mBeatColumns[i] = new BeatColumn(this, (int)i);
}

void Beats::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      mBeatColumns[i]->CreateUIControls();
}

Beats::~Beats()
{
   TheTransport->RemoveListener(this);
}

void Beats::Process(double time)
{
   PROFILER(Beats);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;
   
   int numChannels = 2;

   ComputeSliders(0);
   SyncOutputBuffer(numChannels);
   mWriteBuffer.SetNumActiveChannels(numChannels);

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   mWriteBuffer.Clear();
   
   for (BeatColumn* column : mBeatColumns)
      column->Process(time, &mWriteBuffer, bufferSize);
   
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void Beats::Init()
{
   IDrawableModule::Init();
}

void Beats::DropdownClicked(DropdownList* list)
{
}

void Beats::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void Beats::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   for (BeatColumn* column : mBeatColumns)
      column->RadioButtonUpdated(list, oldVal);
}

void Beats::OnTimeEvent(double time)
{
}

void Beats::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int i = 0;
   for (BeatColumn* column : mBeatColumns)
   {
      column->Draw(i * BEAT_COLUMN_WIDTH+3, 5);
      ++i;
   }
}

void Beats::FilesDropped(vector<string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   SampleDropped(x, y, sample);
}

void Beats::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   int numSamples = sample->LengthInSamples();

   if (numSamples <= 0)
      return;

   int column = x / BEAT_COLUMN_WIDTH;

   if (column < (int)mBeatColumns.size())
      mBeatColumns[column]->AddBeat(sample);
}

void Beats::ButtonClicked(ClickButton* button)
{
}

void Beats::CheckboxUpdated(Checkbox *checkbox)
{
}

void Beats::GetModuleDimensions(float& width, float& height)
{
   width = BEAT_COLUMN_WIDTH * (int)mBeatColumns.size();
   height = 0;
   for (size_t i=0; i < mBeatColumns.size(); ++i)
      height = MAX(height, 132+15*MAX(mBeatColumns[i]->GetNumSamples(),1));
}

void Beats::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Beats::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void Beats::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void Beats::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void Beats::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void BeatData::LoadBeat(Sample* sample)
{
   mBeat = sample;
}

void BeatData::RecalcPos(double time, bool doubleTime, int numBars)
{
   float measurePos = TheTransport->GetMeasure(time) % numBars + TheTransport->GetMeasurePos(time);
   float pos = ofMap(measurePos/ numBars, 0, 1, 0, mBeat->LengthInSamples(), true);
   if (doubleTime)
   {
      pos *= 2;
      if (pos >= mBeat->LengthInSamples())
         pos -= mBeat->LengthInSamples();
   }
   mBeat->SetPlayPosition((int)pos);
}

BeatColumn::BeatColumn(Beats* owner, int index)
: mOwner(owner)
, mVolume(0)
, mSampleIndex(0)
, mIndex(index)
, mFilter(0)
, mDoubleTime(false)
, mNumBars(1)
, mPan(0)
{
   for (size_t i=0; i<mLowpass.size(); ++i)
      mLowpass[i].SetFilterType(kFilterType_Lowpass);
   for (size_t i = 0; i < mHighpass.size(); ++i)
      mHighpass[i].SetFilterType(kFilterType_Highpass);
}

BeatColumn::~BeatColumn()
{
   mSelector->Delete();
   mVolumeSlider->Delete();
   for (size_t i = 0; i < mSamples.size(); ++i)
      delete mSamples[i];
}

void BeatColumn::Process(double time, ChannelBuffer* buffer, int bufferSize)
{
   Sample* beat = mBeatData.mBeat;
   if (beat)
   {
      float volSq = mVolume * mVolume * .25f;
      
      float speed = beat->LengthInSamples() * gInvSampleRateMs / TheTransport->MsPerBar() / mNumBars;
      if (mDoubleTime)
         speed *= 2;
      mBeatData.RecalcPos(time, mDoubleTime, mNumBars);
      beat->SetRate(speed);
      
      int numChannels = 2;
      gWorkChannelBuffer.SetNumActiveChannels(numChannels);
      if (beat->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
      {
         mFilterRamp.Start(time,mFilter,time+10);
         
         for (int ch = 0; ch < numChannels; ++ch)
         {
            float panGain = ch == 0 ? GetLeftPanGain(mPan) : GetRightPanGain(mPan);
            double time = gTime;
            for (int i = 0; i < bufferSize; ++i)
            {
               float filter = mFilterRamp.Value(time);

               mLowpass[ch].SetFilterParams(ofMap(sqrtf(ofClamp(-filter, 0, 1)), 0, 1, 6000, 80), sqrt(2) / 2);
               mHighpass[ch].SetFilterParams(ofMap(ofClamp(filter, 0, 1), 0, 1, 10, 6000), sqrt(2) / 2);

               const float crossfade = .1f;
               float normalAmount = ofClamp(1 - fabsf(filter / crossfade), 0, 1);
               float lowAmount = ofClamp(-filter / crossfade, 0, 1);
               float highAmount = ofClamp(filter / crossfade, 0, 1);

               int sampleChannel = ch;
               if (beat->NumChannels() == 1)
                  sampleChannel = 0;
               float normal = gWorkChannelBuffer.GetChannel(sampleChannel)[i];
               float lowPassed = mLowpass[ch].Filter(normal);
               float highPassed = mHighpass[ch].Filter(normal);
               float sample = normal * normalAmount + lowPassed * lowAmount + highPassed * highAmount;

               sample *= volSq * panGain;
               buffer->GetChannel(ch)[i] += sample;
               time += gInvSampleRateMs;
            }
         }
      }
   }
}

void BeatColumn::Draw(int x, int y)
{
   if (mBeatData.mBeat)
   {
      int w = BEAT_COLUMN_WIDTH - 6;
      int h = 35;

      ofPushMatrix();
      ofTranslate(x, y);
      DrawAudioBuffer(w, h, mBeatData.mBeat->Data(), 0, mBeatData.mBeat->LengthInSamples(), mBeatData.mBeat->GetPlayPosition());

      /*//frequency response
      ofSetColor(52, 204, 235);
      ofSetLineWidth(3);
      ofBeginShape();
      const int kPixelStep = 2;
      bool updateFrequencyResponseGraph = false;
      if (mNeedToUpdateFrequencyResponseGraph)
      {
         updateFrequencyResponseGraph = true;
         mNeedToUpdateFrequencyResponseGraph = false;
      }
      for (int x = 0; x < w + kPixelStep; x += kPixelStep)
      {
         float response = 1;
         float freq = FreqForPos(x / w);
         if (freq < gSampleRate / 2)
         {
            int responseGraphIndex = x / kPixelStep;
            if (updateFrequencyResponseGraph || responseGraphIndex >= mFrequencyResponse.size())
            {
               for (auto& filter : mFilters)
               {
                  if (filter.mEnabled)
                     response *= filter.mFilter[0].GetMagnitudeResponseAt(freq);
               }
               if (responseGraphIndex < mFrequencyResponse.size())
                  mFrequencyResponse[responseGraphIndex] = response;
            }
            else
            {
               response = mFrequencyResponse[responseGraphIndex];
            }
            ofVertex(x, (.5f - .666f * log10(response)) * h);
         }
      }
      ofEndShape(false);*/

      ofPopMatrix();
   }

   mVolumeSlider->SetPosition(x, y + 40);
   mVolumeSlider->Draw();
   mFilterSlider->SetPosition(x, y + 56);
   mFilterSlider->Draw();
   mPanSlider->SetPosition(x, y + 72);
   mPanSlider->Draw();
   mDoubleTimeCheckbox->SetPosition(x, y + 88);
   mDoubleTimeCheckbox->Draw();
   mNumBarsSlider->SetPosition(x, y + 104);
   mNumBarsSlider->Draw();
   mSelector->SetPosition(x, y + 120);
   mSelector->Draw();
}

void BeatColumn::CreateUIControls()
{
   int controlWidth = BEAT_COLUMN_WIDTH - 6;
   mVolumeSlider = new FloatSlider(mOwner,("volume"+ofToString(mIndex)).c_str(),0,0, controlWidth,15,&mVolume,0,1.5f,2);
   mFilterSlider = new FloatSlider(mOwner,("filter"+ofToString(mIndex)).c_str(),0,0, controlWidth,15,&mFilter,-1,1,2);
   mPanSlider = new FloatSlider(mOwner, ("pan" + ofToString(mIndex)).c_str(), 0, 0, controlWidth, 15, &mPan, -1, 1, 2);
   mDoubleTimeCheckbox = new Checkbox(mOwner,("double"+ofToString(mIndex)).c_str(),0,0,&mDoubleTime);
   mNumBarsSlider = new IntSlider(mOwner, ("bars" + ofToString(mIndex)).c_str(), 0, 0, controlWidth, 15, &mNumBars, 1, 8);
   mSelector = new RadioButton(mOwner, ("selector" + ofToString(mIndex)).c_str(), 0, 0, &mSampleIndex);
   
   mSelector->SetForcedWidth(controlWidth);
   mSelector->AddLabel("none", 0);
}

void BeatColumn::AddBeat(Sample* sample)
{
   if (mSamples.size() == 0)
      mSelector->Clear();

   mSelector->AddLabel(sample->Name().c_str(), mSelector->GetNumValues());
   Sample* newSample = new Sample();
   newSample->CopyFrom(sample);
   mSamples.push_back(newSample);

   if (mSamples.size() == 1)
      mBeatData.LoadBeat(mSamples[0]);
}

void BeatColumn::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mSelector)
   {
      mBeatData.LoadBeat(mSamples[mSampleIndex]);
   }
}


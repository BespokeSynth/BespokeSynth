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
: mBank(nullptr)
, mRows(4)
, mSampleBankCable(nullptr)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);
}

void Beats::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mSampleBankCable = new PatchCableSource(this, kConnectionType_Special);
   mSampleBankCable->SetManualPosition(8, 8);
   mSampleBankCable->AddTypeFilter("samplebank");
   AddPatchCableSource(mSampleBankCable);
}

Beats::~Beats()
{
   delete[] mWriteBuffer;
   TheTransport->RemoveListener(this);
}

void Beats::Process(double time)
{
   PROFILER(Beats);

   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   
   for (BeatColumn* column : mBeatColumns)
      column->Process(time, mWriteBuffer, bufferSize);
   
   Add(out, mWriteBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
}

void Beats::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mSampleBankCable)
   {
      if (mBank)
         mBank->RemoveListener(this);
      mBank = dynamic_cast<SampleBank*>(mSampleBankCable->GetTarget());
      if (mBank)
         mBank->AddListener(this);
   }
}

void Beats::OnSamplesLoaded(SampleBank* bank)
{
   mBeatColumns.clear();
   if (mBank)
   {
      const vector<SampleInfo>& samples = mBank->GetSamples();
      for (int i=0; i<samples.size(); ++i)
      {
         if (i % mRows == 0)
         {
            mBeatColumns.push_back(new BeatColumn(this, i/mRows));
            mBeatColumns[i/mRows]->CreateUIControls();
         }
         mBeatColumns[i/mRows]->AddBeat(samples[i].mSample);
      }
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
   
   int i=0;
   for (BeatColumn* column : mBeatColumns)
   {
      column->Draw(i*110,5);
      ++i;
   }
}

const SampleInfo* Beats::GetSampleInfo(int columnIdx, int sampleIdx)
{
   const vector<SampleInfo>& samples = mBank->GetSamples();
   int lookup = columnIdx*mRows+sampleIdx-1;
   if (sampleIdx > 0 && lookup < samples.size())
      return &samples[lookup];
   else
      return nullptr;
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
   height = 96+15*(mRows+1);
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
   mModuleSaveData.LoadString("samplebank", moduleInfo, "", FillDropdown<SampleBank*>);
   mModuleSaveData.LoadInt("rows", moduleInfo, 4, 1, 8);
   
   SetUpFromSaveData();
}

void Beats::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["samplebank"] = mBank ? mBank->Name() : "";
}

void Beats::SetUpFromSaveData()
{
   mRows = mModuleSaveData.GetInt("rows");
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mSampleBankCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("samplebank"),false));
}

void BeatData::LoadBeat(const SampleInfo* info)
{
   if (info)
   {
      mBeat = info->mSample;
      mNumBars = info->mSample->GetNumBars();
   }
   else
   {
      mBeat = nullptr;
      mNumBars = 1;
   }
}

void BeatData::RecalcPos(double time, bool doubleTime)
{
   float measurePos = TheTransport->GetMeasure(time) % mNumBars + TheTransport->GetMeasurePos(time);
   float pos = ofMap(measurePos/mNumBars, 0, 1, 0, mBeat->LengthInSamples(), true);
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
{
   mLowpass.SetFilterType(kFilterType_Lowpass);
   mHighpass.SetFilterType(kFilterType_Highpass);
}

BeatColumn::~BeatColumn()
{
   mSelector->Delete();
   mVolumeSlider->Delete();
}

void BeatColumn::Process(double time, float* buffer, int bufferSize)
{
   Sample* beat = mBeatData.mBeat;
   if (beat)
   {
      float volSq = mVolume * mVolume * .25f;
      
      float speed = beat->LengthInSamples() * gInvSampleRateMs / TheTransport->MsPerBar() / mBeatData.mNumBars;
      if (mDoubleTime)
         speed *= 2;
      mBeatData.RecalcPos(time, mDoubleTime);
      beat->SetRate(speed);
      
      //TODO(Ryan) multichannel
      gWorkChannelBuffer.SetNumActiveChannels(1);
      if (beat->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
      {
         mFilterRamp.Start(mFilter,10);
         
         double time = gTime;
         for (int i=0; i<bufferSize; ++i)
         {
            float filter = mFilterRamp.Value(time);
            
            mLowpass.SetFilterParams(ofMap(sqrtf(ofClamp(-filter,0,1)),0,1,6000,80), 1);
            mHighpass.SetFilterParams(ofMap(ofClamp(filter,0,1),0,1,10,6000), 1);
            
            const float crossfade = .1f;
            float normalAmount = ofClamp(1 - fabsf(filter/crossfade),0,1);
            float lowAmount = ofClamp(-filter/crossfade,0,1);
            float highAmount = ofClamp(filter/crossfade,0,1);
            
            float normal = gWorkChannelBuffer.GetChannel(0)[i];
            float lowPassed = mLowpass.Filter(normal);
            float highPassed = mHighpass.Filter(normal);
            float sample = normal * normalAmount + lowPassed * lowAmount + highPassed * highAmount;
            
            sample *= volSq;
            buffer[i] += sample;
            time += gInvSampleRateMs;
         }
      }
   }
}

void BeatColumn::Draw(int x, int y)
{
   if (mBeatData.mBeat)
   {
      ofPushMatrix();
      ofTranslate(x, y);
      DrawAudioBuffer(100, 35, mBeatData.mBeat->Data(), 0, mBeatData.mBeat->LengthInSamples(), mBeatData.mBeat->GetPlayPosition());
      ofPopMatrix();
   }
   mFilterSlider->SetPosition(x,y+40);
   mFilterSlider->Draw();
   mVolumeSlider->SetPosition(x,y+56);
   mVolumeSlider->Draw();
   mDoubleTimeCheckbox->SetPosition(x, y+72);
   mDoubleTimeCheckbox->Draw();
   mSelector->SetPosition(x,y+88);
   mSelector->Draw();
}

void BeatColumn::CreateUIControls()
{
   mSelector = new RadioButton(mOwner,("selector"+ofToString(mIndex)).c_str(),0,0,&mSampleIndex);
   mVolumeSlider = new FloatSlider(mOwner,("volume"+ofToString(mIndex)).c_str(),0,0,100,15,&mVolume,0,1.5f,2);
   mFilterSlider = new FloatSlider(mOwner,("filter"+ofToString(mIndex)).c_str(),0,0,100,15,&mFilter,-1,1,2);
   mDoubleTimeCheckbox = new Checkbox(mOwner,("double"+ofToString(mIndex)).c_str(),0,0,&mDoubleTime);
   
   mSelector->AddLabel("none", 0);
}

void BeatColumn::AddBeat(Sample* sample)
{
   mSelector->AddLabel(sample->Name(), mSelector->GetNumValues());
}

void BeatColumn::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mSelector)
   {
      mBeatData.LoadBeat(mOwner->GetSampleInfo(mIndex, mSampleIndex));
   }
}


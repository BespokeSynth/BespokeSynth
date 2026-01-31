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
//  Beats.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/2/14.
//
//

#include "Beats.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

Beats::Beats()
: mWriteBuffer(gBufferSize)
{
   mBeatColumns.resize(1);
   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      mBeatColumns[i] = new BeatColumn(this, (int)i);
}

void Beats::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);
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

void Beats::PlayNote(NoteMessage note)
{
   for (BeatColumn* column : mBeatColumns)
      column->PlayNote(note);
}

void Beats::DropdownClicked(DropdownList* list)
{
}

void Beats::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void Beats::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
   for (BeatColumn* column : mBeatColumns)
      column->RadioButtonUpdated(list, oldVal, time);
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
      if (i == mHighlightColumn)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(255, 255, 255, 40);
         float width, height;
         GetModuleDimensions(width, height);
         ofRect(i * BEAT_COLUMN_WIDTH + 1, 3, BEAT_COLUMN_WIDTH - 2, height - 6);
         ofPopStyle();
      }
      column->Draw(i * BEAT_COLUMN_WIDTH + 3, 5);
      ++i;
   }
}

void Beats::FilesDropped(std::vector<std::string> files, int x, int y)
{
   for (auto& file : files)
   {
      Sample* sample = new Sample();
      sample->Read(file.c_str());
      SampleDropped(x, y, sample);
   }

   mHighlightColumn = -1;
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

   mHighlightColumn = -1;
}

bool Beats::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   int column = x / BEAT_COLUMN_WIDTH;

   if (TheSynth->GetHeldSample() != nullptr && column >= 0 && column < (int)mBeatColumns.size())
      mHighlightColumn = column;
   else
      mHighlightColumn = -1;

   return false;
}

void Beats::ClearSamples()
{
   for (BeatColumn* column : mBeatColumns)
      column->ClearSamples();
}

void Beats::ButtonClicked(ClickButton* button, double time)
{
   for (BeatColumn* column : mBeatColumns)
      column->ButtonClicked(button, time);
}

void Beats::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void Beats::GetModuleDimensions(float& width, float& height)
{
   width = BEAT_COLUMN_WIDTH * (int)mBeatColumns.size();
   height = 0;
   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      height = MAX(height, 143 + 15 * (mBeatColumns[i]->GetNumSamples() + 1));
}

void Beats::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Beats::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void Beats::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Beats::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void Beats::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void Beats::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mBeatColumns.size();
   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      mBeatColumns[i]->SaveState(out);
}

void Beats::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int numColumns;
   in >> numColumns;
   if (numColumns != (int)mBeatColumns.size())
   {
      mBeatColumns.resize(numColumns);
      for (size_t i = 0; i < mBeatColumns.size(); ++i)
      {
         mBeatColumns[i] = new BeatColumn(this, (int)i);
         mBeatColumns[i]->CreateUIControls();
      }
   }
   for (size_t i = 0; i < mBeatColumns.size(); ++i)
      mBeatColumns[i]->LoadState(in, rev);
}

bool Beats::LoadOldControl(FileStreamIn& in, std::string& oldName)
{
   if (oldName == "bars" || oldName == "bars0")
   {
      //load dropdown string
      int dummy;
      in >> dummy;
      float numBarsFloat;
      in >> numBarsFloat;
      mLegacyNumBars = (int)numBarsFloat;
      in >> dummy;
      in >> dummy;
      return true;
   }

   return false;
}

void BeatData::RecalcPos(double time)
{
   float measurePos = TheTransport->GetMeasure(time) % mNumBars + TheTransport->GetMeasurePos(time);
   float pos = ofMap(measurePos / mNumBars, 0, 1, 0, mSample->LengthInSamples(), true);
   mSample->SetPlayPosition(pos);
}

BeatColumn::BeatColumn(Beats* owner, int index)
: mOwner(owner)
, mIndex(index)
{
   for (size_t i = 0; i < mLowpass.size(); ++i)
      mLowpass[i].SetFilterType(kFilterType_Lowpass);
   for (size_t i = 0; i < mHighpass.size(); ++i)
      mHighpass[i].SetFilterType(kFilterType_Highpass);
}

BeatColumn::~BeatColumn()
{
   mSelector->Delete();
   mVolumeSlider->Delete();
   for (size_t i = 0; i < mSamples.size(); ++i)
      delete mSamples[i].mSample;
}

void BeatColumn::Process(double time, ChannelBuffer* buffer, int bufferSize)
{
   if (mSampleIndex != -1 && mSamples[mSampleIndex].mSample)
   {
      float volSq = mVolume * mVolume * .25f * mSamples[mSampleIndex].mVolume;
      Sample* beat = mSamples[mSampleIndex].mSample;

      float speed = (beat->LengthInSamples() / beat->GetSampleRateRatio()) * gInvSampleRateMs / TheTransport->MsPerBar() / mSamples[mSampleIndex].mNumBars;
      mSamples[mSampleIndex].RecalcPos(time);
      beat->SetRate(speed);

      int numChannels = 2;
      gWorkChannelBuffer.SetNumActiveChannels(numChannels);
      if (beat->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
      {
         mFilterRamp.Start(time, mFilter, time + 10);

         for (int ch = 0; ch < numChannels; ++ch)
         {
            float panGain = ch == 0 ? GetLeftPanGain(mPan) : GetRightPanGain(mPan);
            double channelTime = time;
            for (int i = 0; i < bufferSize; ++i)
            {
               float filter = mFilterRamp.Value(channelTime);

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
               channelTime += gInvSampleRateMs;
            }
         }
      }
   }
}

void BeatColumn::Draw(int x, int y)
{
   if (mSampleIndex != -1 && mSamples[mSampleIndex].mSample)
   {
      int w = BEAT_COLUMN_WIDTH - 6;
      int h = 35;

      ofPushMatrix();
      ofTranslate(x, y);
      DrawAudioBuffer(w, h, mSamples[mSampleIndex].mSample->Data(), 0, mSamples[mSampleIndex].mSample->LengthInSamples(), mSamples[mSampleIndex].mSample->GetPlayPosition(), mSamples[mSampleIndex].mVolume);

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
   ofPushStyle();
   ofSetColor(ofColor::white, 10);
   ofFill();
   ofRect(x, y + 93, BEAT_COLUMN_WIDTH - 6, 34);
   ofPopStyle();
   mClipVolumeSlider->PositionTo(mPanSlider, kAnchor_Below_Padded);
   mClipVolumeSlider->SetShowing(mSampleIndex != -1);
   mClipVolumeSlider->Draw();
   mClipNumBarsSlider->PositionTo(mClipVolumeSlider, kAnchor_Below);
   mClipNumBarsSlider->SetShowing(mSampleIndex != -1);
   mClipNumBarsSlider->Draw();
   mDeleteButton->PositionTo(mClipNumBarsSlider, kAnchor_Right_Padded);
   mDeleteButton->SetShowing(mSampleIndex != -1);
   mDeleteButton->Draw();
   mSelector->PositionTo(mClipNumBarsSlider, kAnchor_Below_Padded);
   mSelector->Draw();
}

void BeatColumn::CreateUIControls()
{
   std::string suffix = "";
   if (mOwner->GetNumColumns() > 1)
      suffix = ofToString(mIndex);

   int controlWidth = BEAT_COLUMN_WIDTH - 6;
   mVolumeSlider = new FloatSlider(mOwner, ("volume" + suffix).c_str(), 0, 0, controlWidth, 15, &mVolume, 0, 1.5f, 2);
   mFilterSlider = new FloatSlider(mOwner, ("filter" + suffix).c_str(), 0, 0, controlWidth, 15, &mFilter, -1, 1, 2);
   mPanSlider = new FloatSlider(mOwner, ("pan" + suffix).c_str(), 0, 0, controlWidth, 15, &mPan, -1, 1, 2);
   mSelector = new RadioButton(mOwner, ("selector" + suffix).c_str(), 0, 0, &mSampleIndex);
   mDeleteButton = new ClickButton(mOwner, ("delete" + suffix).c_str(), 0, 0);
   mClipVolumeSlider = new FloatSlider(mOwner, ("clip volume" + suffix).c_str(), 0, 0, controlWidth, 15, &mDummyClipVolume, 0, 2, 2);
   mClipNumBarsSlider = new IntSlider(mOwner, ("clip bars" + suffix).c_str(), 0, 0, 95, 15, &mDummyClipNumBars, 1, 8);

   mSelector->SetForcedWidth(controlWidth);

   UpdateRadioButtonLabels();
}

void BeatColumn::AddBeat(Sample* sample)
{
   BeatData beatData;
   beatData.mSample = new Sample();
   beatData.mSample->CopyFrom(sample);
   float lengthSeconds = sample->LengthInSamples() / sample->GetOriginalSampleRate();
   const float kMinTempo = 80;
   int numBars = 1;
   while (true)
   {
      float effectiveTempo = (4 * numBars) / (lengthSeconds / 60.0f);
      if (effectiveTempo >= kMinTempo)
         break;
      else
         numBars *= 2;
   }
   beatData.mNumBars = numBars;
   mSamples.push_back(beatData);

   UpdateRadioButtonLabels();
   UpdateClipSliders();
}

void BeatColumn::UpdateClipSliders()
{
   if (mSampleIndex != -1 && mSampleIndex < (int)mSamples.size())
   {
      mClipVolumeSlider->SetVar(&mSamples[mSampleIndex].mVolume);
      mClipNumBarsSlider->SetVar(&mSamples[mSampleIndex].mNumBars);
   }
   else
   {
      mClipVolumeSlider->SetVar(&mDummyClipVolume);
      mClipNumBarsSlider->SetVar(&mDummyClipNumBars);
   }
}


void BeatColumn::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
   if (list == mSelector)
   {
      UpdateClipSliders();
   }
}

void BeatColumn::ButtonClicked(ClickButton* button, double time)
{
   if (button == mDeleteButton)
   {
      if (mSampleIndex >= 0 && mSampleIndex < (int)mSamples.size())
      {
         mSamples.erase(mSamples.begin() + mSampleIndex);
         mSampleIndex = -1;

         UpdateRadioButtonLabels();
      }
   }
}

void BeatColumn::UpdateRadioButtonLabels()
{
   mSelector->Clear();
   mSelector->AddLabel("none", -1);

   for (int i = 0; i < (int)mSamples.size(); ++i)
      mSelector->AddLabel(mSamples[i].mSample->Name().c_str(), i);
}

void BeatColumn::ClearSamples()
{
   mSamples.clear();
   mSelector->Clear();
   mSelector->AddLabel("none", -1);
}

void BeatColumn::PlayNote(const NoteMessage& note)
{
   if (note.pitch >= 0 && note.pitch < static_cast<int>(mSamples.size()) + 1)
      mSelector->SetValue(note.pitch - 1, gTime);
}

void BeatColumn::SaveState(FileStreamOut& out)
{
   out << (int)mSamples.size();
   for (size_t i = 0; i < mSamples.size(); ++i)
   {
      mSamples[i].mSample->SaveState(out);
      out << mSamples[i].mNumBars;
   }
   out << mSampleIndex;
}

void BeatColumn::LoadState(FileStreamIn& in, int rev)
{
   int numSamples;
   in >> numSamples;
   mSamples.resize(numSamples);
   for (size_t i = 0; i < mSamples.size(); ++i)
   {
      mSamples[i].mSample = new Sample();
      mSamples[i].mSample->LoadState(in);
      if (rev >= 3)
         in >> mSamples[i].mNumBars;
      else
         mSamples[i].mNumBars = mOwner->mLegacyNumBars;
   }
   in >> mSampleIndex;

   UpdateRadioButtonLabels();
}

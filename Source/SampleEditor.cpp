//
//  SampleEditor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#include "SampleEditor.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SampleBank.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "Scale.h"

SampleEditor::SampleEditor()
: mVolume(1)
, mVolumeSlider(nullptr)
, mSample(nullptr)
, mSampleIndex(-1)
, mSampleList(nullptr)
, mPlay(false)
, mPlayCheckbox(nullptr)
, mLoop(true)
, mLoopCheckbox(nullptr)
, mCurrentBar(-1)
, mMeasureEarly(0)
, mEditMode(false)
, mEditCheckbox(nullptr)
, mSampleStart(0)
, mSampleStartSlider(nullptr)
, mSampleEnd(1)
, mSampleEndSlider(nullptr)
, mNumBars(1)
, mNumBarsSlider(nullptr)
, mOffset(0)
, mOffsetSlider(nullptr)
, mEditModeStart(nullptr)
, mPadSampleButton(nullptr)
, mWriteButton(nullptr)
, mOriginalBpm(0)
, mBank(nullptr)
, mKeepPitch(false)
, mKeepPitchCheckbox(nullptr)
, mPitchShift(1)
, mPitchShiftSlider(nullptr)
, mSampleBankCable(nullptr)
, mReset(false)
, mTransposition(0)
, mDrawBuffer(0)
{
   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      mPitchShifter[i] = new PitchShifter(1024);
}

void SampleEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",5,20,110,15,&mVolume,0,2);
   mSampleList = new DropdownList(this,"samples",5,40,&mSampleIndex);
   mPlayCheckbox = new Checkbox(this,"play",5,60,&mPlay);
   mLoopCheckbox = new Checkbox(this,"loop",60,60,&mLoop);
   mEditCheckbox = new Checkbox(this,"edit",60,2,&mEditMode);
   mSampleStartSlider = new FloatSlider(this,"start",5,360,900,15,&mSampleStart,0,gSampleRate*200,0);
   mSampleEndSlider = new FloatSlider(this,"end",5,375,900,15,&mSampleEnd,0,gSampleRate*200,0);
   mNumBarsSlider = new IntSlider(this,"num bars",215,3,110,15,&mNumBars,1,100);
   mOffsetSlider = new FloatSlider(this,"off",215,20,110,15,&mOffset,-1,1,4);
   mEditModeStart = new ClickButton(this,"quickstart",215,50);
   mPadSampleButton = new ClickButton(this,"pad end",400,50);
   mWriteButton = new ClickButton(this,"write",600,50);
   mKeepPitchCheckbox = new Checkbox(this,"keep pitch", 120, 20, &mKeepPitch);
   mPitchShiftSlider = new FloatSlider(this,"pitch shift", 110, 60, 95, 15, &mPitchShift, .5f, 2.0f);
   
   mSampleBankCable = new PatchCableSource(this, kConnectionType_Special);
   mSampleBankCable->SetManualPosition(8, 8);
   mSampleBankCable->AddTypeFilter("samplebank");
   AddPatchCableSource(mSampleBankCable);
}

SampleEditor::~SampleEditor()
{
   TheTransport->RemoveListener(this);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      delete mPitchShifter[i];
}

void SampleEditor::Process(double time)
{
   PROFILER(SampleEditor);

   if (!mEnabled || GetTarget() == nullptr || mSample == nullptr)
      return;

   ComputeSliders(0);
   SyncOutputBuffer(mSample->NumChannels());
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume * .25f;

   float speed = float(mSampleEnd-mSampleStart)/mSample->GetSampleRateRatio() * gInvSampleRateMs / TheTransport->MsPerBar() / mNumBars;
   if (mCurrentBar >= 0)
      RecalcPos();
   mSample->SetRate(speed);

   gWorkChannelBuffer.SetNumActiveChannels(mSample->NumChannels());
   if (mSample->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
      {
         float pitchShift = mPitchShift;
         if (mKeepPitch)
            pitchShift *= mOriginalBpm / TheTransport->GetTempo();
         if (mTransposition != 0)
            pitchShift *= TheScale->PitchToFreq(24 + mTransposition) / TheScale->PitchToFreq(24);
         if (pitchShift != 1)
         {
            mPitchShifter[ch]->SetRatio(pitchShift);
            mPitchShifter[ch]->Process(gWorkChannelBuffer.GetChannel(ch), bufferSize);
         }
         
         Mult(gWorkChannelBuffer.GetChannel(ch), volSq, bufferSize);
         Add(GetTarget()->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      }
   }
   else
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
         GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, ch);
   }
}

void SampleEditor::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mSampleBankCable)
   {
      mBank = dynamic_cast<SampleBank*>(mSampleBankCable->GetTarget());
      
      UpdateSampleList();
   }
}

void SampleEditor::UpdateSampleList()
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

void SampleEditor::DropdownClicked(DropdownList* list)
{
   if (list == mSampleList)
   {
      UpdateSampleList();
   }
}

void SampleEditor::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mSampleList)
   {
      UpdateSample();
   }
}

void SampleEditor::UpdateSample()
{
   const SampleInfo& info = mBank->GetSampleInfo(mSampleIndex);
   mSample = info.mSample;
   mSample->Reset();
   if (info.mOffset < 0)
      mMeasureEarly = 1;
   else
      mMeasureEarly = 0;
   mOffset = info.mOffset;
   mSampleStart = 0;
   mSampleEnd = mSample->LengthInSamples();
   mNumBars = mSample->GetNumBars();
   mSampleStartSlider->SetExtents(0,mSample->LengthInSamples());
   mSampleEndSlider->SetExtents(0,mSample->LengthInSamples());
   TheTransport->UpdateListener(this, kInterval_1n, OffsetInfo(-mOffset, false));
   mCurrentBar = mNumBars;
   mVolume = 1;
   mLoop = info.mType != "vox";
   mPlay = false;
   
   mSample->LockDataMutex(true);
   mDrawBufferLength = mSample->LengthInSamples();
   mDrawBuffer.Resize(mDrawBufferLength);
   mDrawBuffer.CopyFrom(mSample->Data());
   mSample->LockDataMutex(false);
   
   UpdateBPM();
}

void SampleEditor::ButtonClicked(ClickButton *button)
{
   if (button == mEditModeStart)
   {
      mCurrentBar = mNumBars-1;
      TheTransport->SetMeasure(3);
      TheTransport->SetMeasurePos(.5f);
      mPlay = true;
   }
   if (button == mPadSampleButton)
   {
      if (mSample)
      {
         mSample->PadBack(int(.1f*gSampleRate));
         UpdateSample();
      }
   }
   if (button == mWriteButton)
   {
      if (mSample)
      {
         mSample->ClipTo(mSampleStart, mSampleEnd);
         mSample->Write();
         mSampleStart = 0;
         mSampleEnd = mSample->LengthInSamples();

         mSampleStartSlider->SetExtents(0,mSampleEnd);
         mSampleEndSlider->SetExtents(0,mSampleEnd);

         const SampleInfo& info = mBank->GetSampleInfo(mSampleIndex);
         printf("\"%s\" %d %f %f %s\n", mSample->GetReadPath(), mNumBars, mOffset, mVolume, info.mType.c_str());
      }
   }
}

void SampleEditor::OnTimeEvent(double time)
{
   if (mPlay && mSample)
   {
      if (mCurrentBar >= 0)
         ++mCurrentBar;
      if (mReset)
      {
         if (TheTransport->GetMeasure(time) % 4 == 0)
            mCurrentBar = 0;
         mReset = false;
      }
      if ((mCurrentBar > mNumBars && (TheTransport->GetMeasure(time)+mMeasureEarly) % MIN(4,mNumBars) == 0) ||
          (mCurrentBar == mNumBars && mLoop))
      {
         mSample->Play(time, 1, mSampleStart, mSampleEnd);
         mCurrentBar = 0;
      }

      if (mCurrentBar == mNumBars && !mLoop)
         mPlay = false;
   }
}

void SampleEditor::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (mEditMode && y > 80 && y < 350 && mPlay)
   {
      float clickPos = ofMap(x,5,905,0,1,true);
      mCurrentBar = int(clickPos*mNumBars);
   }
}

void SampleEditor::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
   mSampleList->Draw();
   mPlayCheckbox->Draw();
   mLoopCheckbox->Draw();
   mEditCheckbox->Draw();
   mKeepPitchCheckbox->Draw();
   mPitchShiftSlider->Draw();

   DrawTextNormal("Original BPM:"+ofToString(mOriginalBpm,1),100,12);

   if (mSample)
   {
      if (!mEditMode)
      {
         ofPushMatrix();
         ofTranslate(5,80);
         DrawAudioBuffer(200, 40, &mDrawBuffer, 0, mDrawBufferLength, mPlayPosition);
         ofPopMatrix();
      }
      else
      {
         ofPushMatrix();
         ofTranslate(5,80);
         ofPushStyle();

         int width = 900;
         int height = 310;
         int length = mSample->LengthInSamples();
         mSample->LockDataMutex(true);
         //TODO(Ryan) multichannel
         const float* buffer = mSample->Data()->GetChannel(0);
         int pos = mSample->GetPlayPosition();

         ofSetLineWidth(1);
         ofFill();
         ofSetColor(100,100,100);
         ofRect(0, 0, width, height);
         ofNoFill();
         ofSetColor(0,0,0);
         ofRect(0, 0, width, height);

         for (int i = 0; i < width; i++)
         {
            int position =  ofMap(i, 0, width, 0, length, true);
            int mag = sqrtf(fabs(buffer[position]))*height/2 + 1;
            if (mag > height/2)
            {
               ofSetColor(255,0,0);
               mag = height/2;
            }
            else
            {
               ofSetColor(0, 0, 0);
            }
            ofLine(i, height/2+mag, i, height/2-mag);
         }
         
         mSample->LockDataMutex(false);

         int start = ofMap(mSampleStart, 0, length, 0, width, true);
         int end = ofMap(mSampleEnd, 0, length, 0, width, true);

         for (int i = 0; i < mNumBars; i++)
         {
            float barSpacing = float(end-start)/mNumBars;
            int x =  barSpacing * i + start;
            x += barSpacing * -mOffset;
            ofSetColor(255,255,0);
            ofLine(x, 0, x, height);
         }

         ofSetColor(255,0,0);
         ofLine(start,0,start,height);
         ofLine(end,0,end,height);

         ofSetColor(0,255,0);
         int position =  ofMap(pos, 0, length, 0, width, true);
         ofLine(position,0,position,height);

         ofPopStyle();
         ofPopMatrix();

         mSampleStartSlider->Draw();
         mSampleEndSlider->Draw();
         mNumBarsSlider->Draw();
         mOffsetSlider->Draw();
         mEditModeStart->Draw();
         mPadSampleButton->Draw();
         mWriteButton->Draw();
         if (mSample)
            DrawTextNormal(ofToString(mSample->GetPlayPosition()),335,50);
      }
   }
}

void SampleEditor::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mPlayCheckbox)
   {
      mCurrentBar = mNumBars;
      if (mSample)
         mSample->Reset();
   }
}

void SampleEditor::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = 910;
      height = 400;
   }
   else
   {
      width = 210;
      height = 125;
   }
}

void SampleEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
   {
      TheTransport->UpdateListener(this, kInterval_1n, OffsetInfo(-mOffset, false));
   }
   if (slider == mSampleEndSlider)
   {
      if (mSample && mSampleEnd > mSample->LengthInSamples())
         mSampleEnd = mSample->LengthInSamples();
   }
}

void SampleEditor::RecalcPos()
{
   float measurePos = -mOffset + mCurrentBar + TheTransport->GetMeasurePos(gTime);
   if (TheTransport->GetMeasurePos(gTime) > 1+mOffset)
      measurePos -= 1;
   int pos = ofMap(measurePos/mNumBars, 0, 1, mSampleStart, mSampleEnd, true);
   mPlayPosition = pos;
   if (mSample)
      mSample->SetPlayPosition(pos);
}

void SampleEditor::UpdateBPM()
{
   float sampleRate = gSampleRate;
   if (mSample)
      sampleRate *= mSample->GetSampleRateRatio();
   float seconds = (mSampleEnd - mSampleStart) / sampleRate;
   float secondsPerBar = seconds / mNumBars;
   float beatsPerBar = TheTransport->GetTimeSigTop();
   mOriginalBpm = beatsPerBar / secondsPerBar * 60;
}

void SampleEditor::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumBarsSlider)
   {
      UpdateBPM();
   }
}

void SampleEditor::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mSample)
   {
      mPlay = false;
      mCurrentBar = -1;
      if (pitch == 16)
      {
         mSample->Reset();
      }
      else if (pitch >= 0 && pitch < 16 && velocity > 0)
      {
         int slice = (pitch/8)*8 + 7-(pitch%8);
         int barLength = (mSampleEnd - mSampleStart) / mNumBars;
         int position = -mOffset*barLength + (barLength/4)*slice + mSampleStart;
         mSample->Play(time, 1, position);
      }
   }
}

void SampleEditor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("samplebank", moduleInfo,"",FillDropdown<SampleBank*>);

   SetUpFromSaveData();
}

void SampleEditor::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["samplebank"] = mBank ? mBank->Name() : "";
}

void SampleEditor::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mSampleBankCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("samplebank"),false));
}


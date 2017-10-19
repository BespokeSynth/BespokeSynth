//
//  SamplePlayer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#include "SamplePlayer.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SampleBank.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "Scale.h"

SamplePlayer::SamplePlayer()
: mVolume(1)
, mVolumeSlider(NULL)
, mSample(NULL)
, mSampleIndex(-1)
, mSampleList(NULL)
, mPlay(false)
, mPlayCheckbox(NULL)
, mLoop(true)
, mLoopCheckbox(NULL)
, mCurrentBar(-1)
, mMeasureEarly(0)
, mEditMode(false)
, mEditCheckbox(NULL)
, mSampleStart(0)
, mSampleStartSlider(NULL)
, mSampleEnd(1)
, mSampleEndSlider(NULL)
, mNumBars(1)
, mNumBarsSlider(NULL)
, mOffset(0)
, mOffsetSlider(NULL)
, mEditModeStart(NULL)
, mPadSampleButton(NULL)
, mWriteButton(NULL)
, mOriginalBpm(0)
, mBank(NULL)
, mKeepPitch(false)
, mKeepPitchCheckbox(NULL)
, mPitchShift(1)
, mPitchShiftSlider(NULL)
, mPitchShifter(1024)
, mSampleBankCable(NULL)
, mReset(false)
, mTransposition(0)
, mDrawBuffer(NULL)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   TheTransport->AddListener(this, kInterval_1n);
}

void SamplePlayer::CreateUIControls()
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

SamplePlayer::~SamplePlayer()
{
   delete[] mWriteBuffer;
   TheTransport->RemoveListener(this);
}

void SamplePlayer::Process(double time)
{
   Profiler profiler("SamplePlayer");

   if (!mEnabled || GetTarget() == NULL || mSample == NULL)
      return;

   ComputeSliders(0);

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume * .25f;

   float speed = float(mSampleEnd-mSampleStart)/mSample->GetSampleRateRatio() * gInvSampleRateMs / TheTransport->MsPerBar() / mNumBars;
   if (mCurrentBar >= 0)
      RecalcPos();
   mSample->SetRate(speed);

   if (mSample->ConsumeData(mWriteBuffer, bufferSize, true))
   {
      float pitchShift = mPitchShift;
      if (mKeepPitch)
         pitchShift *= mOriginalBpm / TheTransport->GetTempo();
      if (mTransposition != 0)
         pitchShift *= TheScale->PitchToFreq(24 + mTransposition) / TheScale->PitchToFreq(24);
      if (pitchShift != 1)
      {
         mPitchShifter.SetRatio(pitchShift);
         mPitchShifter.Process(mWriteBuffer, bufferSize);
      }
      
      Mult(mWriteBuffer, volSq, bufferSize);
      Add(out, mWriteBuffer, bufferSize);
      GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
   }
   else
   {
      GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, 0);
   }
}

void SamplePlayer::PostRepatch(PatchCableSource* cable)
{
   if (cable == mSampleBankCable)
   {
      mBank = dynamic_cast<SampleBank*>(mSampleBankCable->GetTarget());
      
      UpdateSampleList();
   }
}

void SamplePlayer::UpdateSampleList()
{
   if (mBank == NULL)
      return;

   mSampleList->Clear();
   vector<SampleInfo> samples = mBank->GetSamples();
   for (int i=0; i<samples.size(); ++i)
   {
      mSampleList->AddLabel(samples[i].mSample->Name(), i);
   }
}

void SamplePlayer::DropdownClicked(DropdownList* list)
{
   if (list == mSampleList)
   {
      UpdateSampleList();
   }
}

void SamplePlayer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mSampleList)
   {
      UpdateSample();
   }
}

void SamplePlayer::UpdateSample()
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
   TheTransport->UpdateListener(this, kInterval_1n, -mOffset, false);
   mCurrentBar = mNumBars;
   mVolume = 1;
   mLoop = info.mType != "vox";
   mPlay = false;
   
   mSample->LockDataMutex(true);
   delete[] mDrawBuffer;
   mDrawBufferLength = mSample->LengthInSamples();
   mDrawBuffer = new float[mDrawBufferLength];
   BufferCopy(mDrawBuffer, mSample->Data(), mDrawBufferLength);
   mSample->LockDataMutex(false);
   
   UpdateBPM();
}

void SamplePlayer::ButtonClicked(ClickButton *button)
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

void SamplePlayer::OnTimeEvent(int samplesTo)
{
   if (mPlay && mSample)
   {
      if (mCurrentBar >= 0)
         ++mCurrentBar;
      if (mReset)
      {
         if (TheTransport->GetMeasure() % 4 == 0)
            mCurrentBar = 0;
         mReset = false;
      }
      if ((mCurrentBar > mNumBars && (TheTransport->GetMeasure()+mMeasureEarly) % MIN(4,mNumBars) == 0) ||
          (mCurrentBar == mNumBars && mLoop))
      {
         mSample->Play(1,mSampleStart,mSampleEnd);
         mCurrentBar = 0;
      }

      if (mCurrentBar == mNumBars && !mLoop)
         mPlay = false;
   }
}

void SamplePlayer::OnClicked(int x, int y, bool right)
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

void SamplePlayer::DrawModule()
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

   DrawText("Original BPM:"+ofToString(mOriginalBpm,1),100,12);

   if (mSample)
   {
      if (!mEditMode)
      {
         ofPushMatrix();
         ofTranslate(5,80);
         DrawAudioBuffer(200, 40, mDrawBuffer, 0, mDrawBufferLength, mPlayPosition);
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
         const float* buffer = mSample->Data();
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
            DrawText(ofToString(mSample->GetPlayPosition()),335,50);
      }
   }
}

void SamplePlayer::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mPlayCheckbox)
   {
      mCurrentBar = mNumBars;
      if (mSample)
         mSample->Reset();
   }
}

void SamplePlayer::GetModuleDimensions(int& x, int&y)
{
   if (mEditMode)
   {
      x = 910;
      y = 400;
   }
   else
   {
      x = 210;
      y = 125;
   }
}

void SamplePlayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
   {
      TheTransport->UpdateListener(this, kInterval_1n, -mOffset, false);
   }
   if (slider == mSampleEndSlider)
   {
      if (mSample && mSampleEnd > mSample->LengthInSamples())
         mSampleEnd = mSample->LengthInSamples();
   }
}

void SamplePlayer::RecalcPos()
{
   float measurePos = -mOffset + mCurrentBar + TheTransport->GetMeasurePos();
   if (TheTransport->GetMeasurePos() > 1+mOffset)
      measurePos -= 1;
   int pos = ofMap(measurePos/mNumBars, 0, 1, mSampleStart, mSampleEnd, true);
   mPlayPosition = pos;
   if (mSample)
      mSample->SetPlayPosition(pos);
}

void SamplePlayer::UpdateBPM()
{
   float sampleRate = gSampleRate;
   if (mSample)
      sampleRate *= mSample->GetSampleRateRatio();
   float seconds = (mSampleEnd - mSampleStart) / sampleRate;
   float secondsPerBar = seconds / mNumBars;
   float beatsPerBar = TheTransport->GetTimeSigTop();
   mOriginalBpm = beatsPerBar / secondsPerBar * 60;
}

void SamplePlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumBarsSlider)
   {
      UpdateBPM();
   }
}

void SamplePlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
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
         mSample->Play(1,position);
      }
   }
}

void SamplePlayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("samplebank", moduleInfo,"",FillDropdown<SampleBank*>);

   SetUpFromSaveData();
}

void SamplePlayer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["samplebank"] = mBank ? mBank->Name() : "";
}

void SamplePlayer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mSampleBankCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("samplebank"),false));
}


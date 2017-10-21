/*
  ==============================================================================

    SamplePlayer.cpp
    Created: 19 Oct 2017 10:10:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

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
, mVolumeSlider(nullptr)
, mSpeed(1)
, mSpeedSlider(nullptr)
, mSample(nullptr)
, mSampleIndex(-1)
, mSampleList(nullptr)
, mPlay(false)
, mLoop(false)
, mLoopCheckbox(nullptr)
, mBank(nullptr)
, mSampleBankCable(nullptr)
, mDrawBuffer(nullptr)
, mPlayButton(nullptr)
, mPauseButton(nullptr)
, mStopButton(nullptr)
, mScrubbingSample(false)
{
}

void SamplePlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",20,3,90,15,&mVolume,0,2);
   mSpeedSlider = new FloatSlider(this,"speed",-1,-1,90,15,&mSpeed,0,2);
   mSampleList = new DropdownList(this,"samples",5,20,&mSampleIndex);

   mPlayButton = new ClickButton(this,"play",5,40);
   mPauseButton = new ClickButton(this,"pause",-1,-1);
   mStopButton = new ClickButton(this,"stop",-1,-1);
   mLoopCheckbox = new Checkbox(this,"loop",-1,-1,&mLoop);
   
   mSpeedSlider->PositionTo(mVolumeSlider, kAnchorDirection_Right);
   mPauseButton->PositionTo(mPlayButton, kAnchorDirection_Right);
   mStopButton->PositionTo(mPauseButton, kAnchorDirection_Right);
   mLoopCheckbox->PositionTo(mStopButton, kAnchorDirection_Right);
   
   mSampleBankCable = new PatchCableSource(this, kConnectionType_Special);
   mSampleBankCable->SetManualPosition(8, 8);
   mSampleBankCable->AddTypeFilter("samplebank");
   AddPatchCableSource(mSampleBankCable);
}

SamplePlayer::~SamplePlayer()
{
   delete mDrawBuffer;
   if (mOwnsSample)
      delete mSample;
}

void SamplePlayer::Process(double time)
{
   Profiler profiler("SamplePlayer");
   
   if (!mEnabled || GetTarget() == nullptr || mSample == nullptr)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   float volSq = mVolume * mVolume * .25f;
   
   if (mPlay && mSample->ConsumeData(gWorkBuffer, bufferSize, true))
   {
      Mult(gWorkBuffer, volSq, bufferSize);
      Add(out, gWorkBuffer, bufferSize);
      GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, 0);
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
   if (mBank == nullptr)
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
      UpdateSample(mBank->GetSampleInfo(mSampleIndex).mSample, false);
   }
}

void SamplePlayer::FilesDropped(vector<string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   UpdateSample(sample, true);
}

void SamplePlayer::UpdateSample(Sample* sample, bool ownsSample)
{
   if (mOwnsSample)
      delete mSample;
   
   mSample = sample;
   mSample->SetPlayPosition(0);
   mSample->SetLooping(mLoop);
   mSample->SetRate(mSpeed);
   mVolume = 1;
   mPlay = false;
   mOwnsSample = ownsSample;
   
   mSample->LockDataMutex(true);
   delete mDrawBuffer;
   mDrawBuffer = new ChannelBuffer(mSample->LengthInSamples());
   BufferCopy(mDrawBuffer->GetChannel(0), mSample->Data(), mSample->LengthInSamples());
   mSample->LockDataMutex(false);
}

void SamplePlayer::ButtonClicked(ClickButton *button)
{
   if (button == mPlayButton)
      mPlay = true;
   if (button == mPauseButton)
      mPlay = false;
   if (button == mStopButton)
   {
      mPlay = false;
      mSample->SetPlayPosition(0);
   }
}

void SamplePlayer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (y > 60)
   {
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
      mScrubbingSample = true;
   }
}

bool SamplePlayer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   if (mScrubbingSample)
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
   return true;
}

void SamplePlayer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mScrubbingSample = false;
}

float SamplePlayer::GetPlayPositionForMouse(float mouseX) const
{
   return ofMap(mouseX, 5, 205 ,0, mSample->LengthInSamples(), true);
}

void SamplePlayer::DrawModule()
{
   
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mSpeedSlider->Draw();
   mSampleList->Draw();
   mLoopCheckbox->Draw();
   mPlayButton->Draw();
   mPauseButton->Draw();
   mStopButton->Draw();
   
   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(5,60);
      DrawAudioBuffer(200, 60, mDrawBuffer, 0, mDrawBuffer->BufferSize(), mSample->GetPlayPosition());
      ofPopMatrix();
   }
}

void SamplePlayer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mLoopCheckbox)
      mSample->SetLooping(mLoop);
}

void SamplePlayer::GetModuleDimensions(int& x, int&y)
{
   x = 210;
   y = 125;
}

void SamplePlayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mSpeedSlider)
   {
      if (mSample)
         mSample->SetRate(mSpeed);
   }
}

void SamplePlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
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

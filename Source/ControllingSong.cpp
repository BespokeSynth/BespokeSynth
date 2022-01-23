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
//  ControllingSong.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#include "ControllingSong.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Transport.h"
#include "Scale.h"
#include "FollowingSong.h"
#include "IAudioReceiver.h"

ControllingSong::ControllingSong()
: mVolume(.8f)
, mVolumeSlider(nullptr)
, mSongStartTime(0)
, mNeedNewSong(true)
, mLoadingSong(true)
, mCurrentSongIndex(-1)
, mShuffleIndex(0)
, mNextSongButton(nullptr)
, mSongSelector(nullptr)
, mTestBeatOffset(0)
, mTestBeatOffsetSlider(nullptr)
, mPlay(false)
, mPlayCheckbox(nullptr)
, mShuffle(false)
, mShuffleCheckbox(nullptr)
, mPhraseBackButton(nullptr)
, mPhraseForwardButton(nullptr)
, mSpeed(1)
, mSpeedSlider(nullptr)
, mMute(false)
, mMuteCheckbox(nullptr)
{
}

void ControllingSong::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"vol",150,20,100,15,&mVolume,0,1);
   mNextSongButton = new ClickButton(this,"next",350,4);
   mSongSelector = new DropdownList(this,"song",150,4,&mCurrentSongIndex);
   mTestBeatOffsetSlider = new IntSlider(this,"test offset",400,4,100,15,&mTestBeatOffset,-10,10);
   mPlayCheckbox = new Checkbox(this,"play",4,15,&mPlay);
   mShuffleCheckbox = new Checkbox(this,"shuffle",80,15,&mShuffle);
   mPhraseBackButton = new ClickButton(this, " [ ", 350, 20);
   mPhraseForwardButton = new ClickButton(this, " ] ", 390, 20);
   mSpeedSlider = new FloatSlider(this,"speed",450,20,100,15,&mSpeed,0,5);
   mMuteCheckbox = new Checkbox(this,"mute",250,4,&mMute);
}

ControllingSong::~ControllingSong()
{
}

void ControllingSong::Init()
{
   IDrawableModule::Init();
   
   for (int i=0; i<mSongList["songs"].size(); ++i)
   {
      mShuffleList.push_back(i);
      std::string title = mSongList["songs"][i]["name"].asString();
      if (mSongList["songs"][i]["keyroot"].asString() == "X")
         title = "X " + title;
      mSongSelector->AddLabel(title.c_str(), i);
   }
   
   std::shuffle(begin(mShuffleList), end(mShuffleList), std::mt19937(std::random_device()()));
}

void ControllingSong::Poll()
{
   if (mNeedNewSong && gTime > 750)
   {
      int nextSong;
      if (mShuffle)
      {
         nextSong = mShuffleList[mShuffleIndex++];
         
         if (mShuffleIndex == mSongList["songs"].size())
         {
            mShuffleIndex = 0;
            std::shuffle(begin(mShuffleList), end(mShuffleList), std::mt19937(std::random_device()()));
         }
      }
      else
      {
         nextSong = mCurrentSongIndex+1;
         
         if (nextSong == mSongList["songs"].size())
            nextSong = 0;
      }
      
      LoadSong(nextSong);
      
      mNeedNewSong = false;
   }
}

void ControllingSong::LoadSong(int index)
{
   mLoadingSong = true;
   
   mLoadSongMutex.lock();
   
   mCurrentSongIndex = index;
   
   mMidiReader.Read(ofToDataPath(mSongList["songs"][index]["midi"].asString()).c_str());
   
   for (int i=0; i<mSongList["songs"][index]["wavs"].size(); ++i)
   {
      if (i==0)
      {
         mSample.Read(ofToDataPath(mSongList["songs"][index]["wavs"][i].asString()).c_str());
         mSample.SetPlayPosition(0);
      }
      else
      {
         int followIdx = i-1;
         if (followIdx < mFollowSongs.size())
            mFollowSongs[followIdx]->LoadSample(ofToDataPath(mSongList["songs"][index]["wavs"][i].asString()).c_str());
      }
   }
   
   std::string keyroot = mSongList["songs"][index]["keyroot"].asString();
   if (keyroot != "X" && keyroot != "")
   {
      TheScale->SetRoot(PitchFromNoteName(keyroot));
      TheScale->SetScaleType(mSongList["songs"][index]["keytype"].asString());
   }
   
   mMidiReader.SetBeatOffset(mSongList["songs"][index]["beatoffset"].asInt());
   
   mSongStartTime = gTime;
   
   mLoadSongMutex.unlock();
   
   mLoadingSong = false;
}

void ControllingSong::Process(double time)
{
   PROFILER(ControllingSong);
   
   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   float volSq = mVolume * mVolume * .5f;
   
   mSample.SetRate(mSpeed);
   
   for (int i=0; i<mFollowSongs.size(); ++i)
   {
      mFollowSongs[i]->SetPlaybackInfo(mPlay, mSample.GetPlayPosition(), mSpeed, mVolume);
   }
   
   if (!mLoadingSong && mPlay)
   {
      mLoadSongMutex.lock();
      
      double ms = mSample.GetPlayPosition() / mSample.GetSampleRateRatio() / double(gSampleRate) * 1000.0;
      if (ms >= 0)
      {
         TheTransport->SetTempo(MIN(200,mMidiReader.GetTempo(ms)) * mSpeed);
         int measure;
         float measurePos;
         mMidiReader.GetMeasurePos(ms, measure, measurePos);
         TheTransport->SetMeasureTime(measure + measurePos);
      }
      
      gWorkChannelBuffer.SetNumActiveChannels(1);
      if (mSample.ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = gWorkChannelBuffer.GetChannel(0)[i] * volSq;
            if (mMute)
               sample = 0;
            out[i] += sample;
            GetVizBuffer()->Write(sample, 0);
         }
      }
      else
      {
         //mNeedNewSong = true;
         GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, 0);
      }
      mLoadSongMutex.unlock();
   }
   else
   {
      GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, 0);
   }
}

void ControllingSong::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mNextSongButton->Draw();
   mSongSelector->Draw();
   mTestBeatOffsetSlider->Draw();
   mPlayCheckbox->Draw();
   mShuffleCheckbox->Draw();
   mPhraseForwardButton->Draw();
   mPhraseBackButton->Draw();
   mSpeedSlider->Draw();
   mMuteCheckbox->Draw();
   mVolumeSlider->Draw();
   
   if (mCurrentSongIndex != -1)
   {
      ofPushMatrix();
      ofTranslate(10,50);
      if (mCurrentSongIndex != -1)
         DrawTextNormal(mSongList["songs"][mCurrentSongIndex]["name"].asString(),0,-10);
      DrawAudioBuffer(540, 100, mSample.Data(), 0, mSample.LengthInSamples()/mSample.GetSampleRateRatio(), mSample.GetPlayPosition());
      ofPopMatrix();
   }
   
   ofPushStyle();
   float w,h;
   GetDimensions(w,h);
   ofFill();
   ofSetColor(255,255,255,50);
   float beatWidth = w/4;
   ofRect(int(TheTransport->GetMeasurePos(gTime)*4)*beatWidth,0,beatWidth,h);
   ofPopStyle();
}

void ControllingSong::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mSongSelector)
   {
      LoadSong(mCurrentSongIndex);
   }
}

void ControllingSong::ButtonClicked(ClickButton* button)
{
   if (button == mNextSongButton)
      mNeedNewSong = true;
   if (button == mPhraseForwardButton || button == mPhraseBackButton)
   {
      int position = mSample.GetPlayPosition();
      int jumpAmount = TheTransport->GetDuration(kInterval_4) / gInvSampleRateMs * mSample.GetSampleRateRatio();
      if (button == mPhraseBackButton)
         jumpAmount *= -1;
      int newPosition = position + jumpAmount;
      if (newPosition < 0)
         newPosition = 0;
      mSample.SetPlayPosition(newPosition);
   }
}

void ControllingSong::CheckboxUpdated(Checkbox* checkbox)
{
}

void ControllingSong::RadioButtonUpdated(RadioButton* list, int oldVal)
{
}

void ControllingSong::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mTestBeatOffsetSlider)
   {
      mMidiReader.SetBeatOffset(mTestBeatOffset);
   }
}

void ControllingSong::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void ControllingSong::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("songs", moduleInfo);
   
   const Json::Value& follows = moduleInfo["followsongs"];
   for (int i=0; i<follows.size(); ++i)
   {
      std::string follow = follows[i].asString();
      FollowingSong* followSong = dynamic_cast<FollowingSong*>(TheSynth->FindModule(follow));
      if (followSong)
         mFollowSongs.push_back(followSong);
   }
   
   SetUpFromSaveData();
}

void ControllingSong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mSongList.open(ofToDataPath(mModuleSaveData.GetString("songs")));
}

void ControllingSong::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["followsongs"].resize((unsigned int)mFollowSongs.size());
   for (int i=0; i<mFollowSongs.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mFollowSongs[i]);
      moduleInfo["followsongs"][i] = module->Name();
   }
}


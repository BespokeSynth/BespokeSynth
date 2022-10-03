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
//  FollowingSong.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 10/15/14.
//
//

#include "FollowingSong.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Transport.h"
#include "Scale.h"
#include "IAudioReceiver.h"

FollowingSong::FollowingSong()
{
}

void FollowingSong::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMuteCheckbox = new Checkbox(this, "mute", 200, 4, &mMute);
}

FollowingSong::~FollowingSong()
{
}

void FollowingSong::LoadSample(const char* file)
{
   mLoadingSong = true;
   mLoadSongMutex.lock();
   mSample.Read(file);
   mLoadSongMutex.unlock();
   mLoadingSong = false;
}

void FollowingSong::SetPlaybackInfo(bool play, int position, float speed, float volume)
{
   mPlay = play;
   mSample.SetRate(speed);
   mSample.SetPlayPosition(position);
   mVolume = volume;
}

void FollowingSong::Process(double time)
{
   PROFILER(FollowingSong);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume * .5f;

   if (!mLoadingSong && mPlay)
   {
      mLoadSongMutex.lock();

      gWorkChannelBuffer.SetNumActiveChannels(1);
      if (mSample.ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
      {
         for (int i = 0; i < bufferSize; ++i)
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
         GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, 0);
      }
      mLoadSongMutex.unlock();
   }
   else
   {
      GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, 0);
   }
}

void FollowingSong::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMuteCheckbox->Draw();

   ofPushMatrix();
   ofTranslate(10, 20);
   DrawTextNormal(mSample.Name(), 100, -10);
   DrawAudioBuffer(540, 100, mSample.Data(), 0, mSample.LengthInSamples() / mSample.GetSampleRateRatio(), mSample.GetPlayPosition());
   ofPopMatrix();

   ofPushStyle();
   float w, h;
   GetDimensions(w, h);
   ofFill();
   ofSetColor(255, 255, 255, 50);
   float beatWidth = w / 4;
   ofRect(int(TheTransport->GetMeasurePos(gTime) * 4) * beatWidth, 0, beatWidth, h);
   ofPopStyle();
}

void FollowingSong::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void FollowingSong::ButtonClicked(ClickButton* button, double time)
{
}

void FollowingSong::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void FollowingSong::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
}

void FollowingSong::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void FollowingSong::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void FollowingSong::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FollowingSong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

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
//  Metronome.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#include "Metronome.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

Metronome::Metronome()
{
}

void Metronome::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, kInterval_4n, OffsetInfo(0, true), false);
}

void Metronome::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "vol", 5, 18, 70, 15, &mVolume, 0, 1);
}

Metronome::~Metronome()
{
   TheTransport->RemoveListener(this);
}

void Metronome::Process(double time)
{
   PROFILER(Metronome);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   for (int i = 0; i < bufferSize; ++i)
   {
      float sample = mOsc.Audio(time, mPhase) * mVolume / 10;
      out[i] += sample;
      GetVizBuffer()->Write(sample, 0);

      mPhase += mPhaseInc;
      while (mPhase > FTWO_PI)
      {
         mPhase -= FTWO_PI;
      }

      time += gInvSampleRateMs;
   }
}

void Metronome::OnTimeEvent(double time)
{
   int step = TheTransport->GetQuantized(time, mTransportListenerInfo);
   if (step == 0)
   {
      mPhaseInc = GetPhaseInc(880);
      mOsc.Start(gTime, 1, 0, 100, 0, 0);
   }
   else if (step == 2)
   {
      mPhaseInc = GetPhaseInc(480);
      mOsc.Start(gTime, 1, 0, 70, 0, 0);
   }
   else
   {
      mPhaseInc = GetPhaseInc(440);
      mOsc.Start(gTime, .8f, 0, 50, 0, 0);
   }
}

void Metronome::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
}

void Metronome::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Metronome::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

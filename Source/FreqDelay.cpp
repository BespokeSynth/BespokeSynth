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
//  FreqDelay.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/10/13.
//
//

#include "FreqDelay.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"

FreqDelay::FreqDelay()
: IAudioProcessor(gBufferSize)
, mDryBuffer(gBufferSize)
{
   AddChild(&mDelayEffect);
   mDelayEffect.SetPosition(5, 30);
   mDelayEffect.SetEnabled(true);
   mDelayEffect.SetDelay(15);
}

void FreqDelay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDryWetSlider = new FloatSlider(this, "dry/wet", 7, 2, 100, 15, &mDryWet, 0, 1);
   mDelayEffect.CreateUIControls();
   mDelayEffect.SetShortMode(true);
}

FreqDelay::~FreqDelay()
{
}

void FreqDelay::Process(double time)
{
   PROFILER(FreqDelay);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   mDryBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = GetBuffer()->BufferSize();

   mDryBuffer.CopyFrom(GetBuffer());
   mDelayEffect.ProcessAudio(time, GetBuffer());

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      Mult(mDryBuffer.GetChannel(ch), (1 - mDryWet), bufferSize);
      Mult(GetBuffer()->GetChannel(ch), mDryWet, bufferSize);
      Add(GetBuffer()->GetChannel(ch), mDryBuffer.GetChannel(ch), bufferSize);
      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void FreqDelay::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      float freq = TheScale->PitchToFreq(note.pitch);
      float ms = 1000 / freq;
      mDelayEffect.SetDelay(ms);
   }
}

void FreqDelay::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mDelayEffect.Draw();
   mDryWetSlider->Draw();
}

void FreqDelay::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void FreqDelay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FreqDelay::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

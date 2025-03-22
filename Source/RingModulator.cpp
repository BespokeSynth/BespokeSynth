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
//  RingModulator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "RingModulator.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"

RingModulator::RingModulator()
: IAudioProcessor(gBufferSize)
, mDryBuffer(gBufferSize)
{
   mModOsc.Start(gTime, 1);
   mFreqRamp.Start(gTime, 220, gTime + mGlideTime);
}

void RingModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFreqSlider = new FloatSlider(this, "freq", 5, 4, 120, 15, &mFreq, 20, 2000);
   mDryWetSlider = new FloatSlider(this, "dry/wet", 5, 20, 120, 15, &mDryWet, 0, 1);
   mVolumeSlider = new FloatSlider(this, "volume", 5, 36, 120, 15, &mVolume, 0, 2);
   mGlideSlider = new FloatSlider(this, "glide", 5, 52, 120, 15, &mGlideTime, 0, 1000);

   mFreqSlider->SetMode(FloatSlider::kLogarithmic);
}

RingModulator::~RingModulator()
{
}

void RingModulator::Process(double time)
{
   PROFILER(RingModulator);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   mDryBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = target->GetBuffer()->BufferSize();

   if (mEnabled)
   {
      mDryBuffer.CopyFrom(GetBuffer());

      for (int i = 0; i < bufferSize; ++i)
      {
         ComputeSliders(i);

         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
            GetBuffer()->GetChannel(ch)[i] *= mModOsc.Audio(time, mPhase);

         float phaseInc = GetPhaseInc(mFreqRamp.Value(time));
         mPhase += phaseInc;
         while (mPhase > FTWO_PI)
         {
            mPhase -= FTWO_PI;
         }

         time += gInvSampleRateMs;
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      if (mEnabled)
      {
         Mult(mDryBuffer.GetChannel(ch), (1 - mDryWet) * mVolume * mVolume, GetBuffer()->BufferSize());
         Mult(GetBuffer()->GetChannel(ch), mDryWet * mVolume * mVolume, GetBuffer()->BufferSize());
         Add(GetBuffer()->GetChannel(ch), mDryBuffer.GetChannel(ch), GetBuffer()->BufferSize());
      }

      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void RingModulator::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mFreqSlider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();

   mGlideSlider->Draw();
}

void RingModulator::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      float freq = TheScale->PitchToFreq(note.pitch);
      mFreqRamp.Start(note.time, freq, note.time + mGlideTime);
      mFreq = freq;
   }
}

void RingModulator::ButtonClicked(ClickButton* button, double time)
{
}

void RingModulator::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void RingModulator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mFreqSlider)
   {
      mFreqRamp.SetValue(mFreq);
   }
}

void RingModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void RingModulator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

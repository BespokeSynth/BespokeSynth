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
/*
  ==============================================================================

    AudioSend.cpp
    Created: 22 Oct 2017 1:23:40pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioSend.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "Checkbox.h"

AudioSend::AudioSend()
: IAudioProcessor(gBufferSize)
, mVizBuffer2(VIZ_BUFFER_SECONDS * gSampleRate)
{
}

void AudioSend::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mAmountSlider = new FloatSlider(this, "amount", 3, 3, 80, 15, &mAmount, 0, 1, 2);
   mCrossfadeCheckbox = new Checkbox(this, "crossfade", mAmountSlider, kAnchor_Below, &mCrossfade);

   float w, h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w / 2 - 15, h + 3);
   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);

   mPatchCableSource2 = new PatchCableSource(this, kConnectionType_Audio);
   mPatchCableSource2->SetManualPosition(w / 2 + 15, h + 3);
   mPatchCableSource2->SetOverrideVizBuffer(&mVizBuffer2);
   mPatchCableSource2->SetManualSide(PatchCableSource::Side::kBottom);
   AddPatchCableSource(mPatchCableSource2);
}

AudioSend::~AudioSend()
{
}

void AudioSend::Process(double time)
{
   PROFILER(AudioSend);

   IAudioReceiver* target0 = GetTarget(0);
   IAudioReceiver* target1 = GetTarget(1);

   if (target0 == nullptr && target1 == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      if (target0)
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            Add(target0->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
            GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
         }

      GetBuffer()->Reset();
      return;
   }

   ComputeSliders(0);
   mVizBuffer2.SetNumChannels(GetBuffer()->NumActiveChannels());

   float* amountBuffer = gWorkBuffer;
   float* dryAmountBuffer = gWorkBuffer + gBufferSize;
   for (int i = 0; i < gBufferSize; ++i)
   {
      ComputeSliders(i);
      amountBuffer[i] = mAmount;
      dryAmountBuffer[i] = 1 - mAmount;
   }

   if (target0)
   {
      gWorkChannelBuffer.CopyFrom(GetBuffer(), GetBuffer()->BufferSize());
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         ChannelBuffer* out = target0->GetBuffer();
         if (mCrossfade)
            Mult(gWorkChannelBuffer.GetChannel(ch), dryAmountBuffer, GetBuffer()->BufferSize());
         Add(out->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   if (target1)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         ChannelBuffer* out2 = target1->GetBuffer();
         Mult(GetBuffer()->GetChannel(ch), amountBuffer, GetBuffer()->BufferSize());
         Add(out2->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         mVizBuffer2.WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void AudioSend::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAmountSlider->Draw();
   mCrossfadeCheckbox->Draw();
}

void AudioSend::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void AudioSend::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("target2", moduleInfo);

   SetUpFromSaveData();
}

void AudioSend::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IClickable* target2 = TheSynth->FindModule(mModuleSaveData.GetString("target2"));
   if (target2)
      mPatchCableSource2->AddPatchCable(target2);
}

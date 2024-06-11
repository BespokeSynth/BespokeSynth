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

    Panner.cpp
    Created: 10 Oct 2017 9:49:17pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Panner.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Panner::Panner()
: IAudioProcessor(gBufferSize)
{
}

void Panner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this, "pan", 5, 20, 110, 15, &mPan, -1, 1);
   mWidenSlider = new FloatSlider(this, "widen", 55, 2, 60, 15, &mWiden, -150, 150, 0);
}

Panner::~Panner()
{
}

void Panner::Process(double time)
{
   PROFILER(Panner);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   if (!mEnabled)
   {
      SyncBuffers();

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   SyncBuffers(2);
   mWidenerBuffer.SetNumChannels(2);

   float* secondChannel;
   if (GetBuffer()->NumActiveChannels() == 1) //panning mono input
   {
      BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
      secondChannel = gWorkBuffer;
   }
   else
   {
      secondChannel = GetBuffer()->GetChannel(1);
   }

   ChannelBuffer* out = target->GetBuffer();

   if (abs(mWiden) > 0)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         mWidenerBuffer.WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      if (mWiden < 0)
         mWidenerBuffer.ReadChunk(secondChannel, GetBuffer()->BufferSize(), abs(mWiden), (GetBuffer()->NumActiveChannels() == 1) ? 0 : 1);
      else
         mWidenerBuffer.ReadChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), abs(mWiden), 0);
   }

   mPanRamp.Start(time, mPan, time + 2);
   for (int i = 0; i < GetBuffer()->BufferSize(); ++i)
   {
      mPan = mPanRamp.Value(time);

      ComputeSliders(i);

      float left = GetBuffer()->GetChannel(0)[i];
      float right = secondChannel[i];
      GetBuffer()->GetChannel(0)[i] = left * ofMap(mPan, 0, 1, 1, 0, true) + right * ofMap(mPan, -1, 0, 1, 0, true);
      secondChannel[i] = right * ofMap(mPan, -1, 0, 0, 1, true) + left * ofMap(mPan, 0, 1, 0, 1, true);

      out->GetChannel(0)[i] += GetBuffer()->GetChannel(0)[i];
      out->GetChannel(1)[i] += secondChannel[i];

      time += gInvSampleRateMs;
   }

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);
   GetVizBuffer()->WriteChunk(secondChannel, GetBuffer()->BufferSize(), 1);

   GetBuffer()->Reset();
}

void Panner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPanSlider->Draw();
   mWidenSlider->Draw();

   GetLeftPanGain(mPan);
   GetRightPanGain(mPan);
}

void Panner::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Panner::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void Panner::ButtonClicked(ClickButton* button, double time)
{
}

void Panner::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void Panner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("pan", moduleInfo, 0, mPanSlider);

   SetUpFromSaveData();
}

void Panner::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetPan(mModuleSaveData.GetFloat("pan"));
}

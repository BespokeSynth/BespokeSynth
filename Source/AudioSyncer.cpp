/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    AudioSyncer.cpp
    Created: 3 May 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioSyncer.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"
#include "Transport.h"

const float mBufferX = 5;
const float mBufferY = 60;
const float mBufferW = 800;
const float mBufferH = 200;

AudioSyncer::AudioSyncer()
: IAudioProcessor(gBufferSize)
, mWriteBuffer(gBufferSize)
, mDelayBuffer(5 * gSampleRate)
{
   mBiquadFilter.SetFilterType(kFilterType_Highpass);
   mBiquadFilter.SetFilterParams(2000, sqrt(2.0f) / 2);
   mBiquadFilter.UpdateFilterCoeff();
}

void AudioSyncer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mPassthroughCheckbox = new Checkbox(this, "passthrough", 5, 5, &mPassthrough);
   mDisplayLengthMsSlider = new FloatSlider(this, "display length", mPassthroughCheckbox, kAnchor_Below, 120, 15, &mDisplayLengthMs, .1f, mDelayBuffer.Size() / gSampleRateMs);
   mLatencyMsSlider = new FloatSlider(this, "latencyMs", mDisplayLengthMsSlider, kAnchor_Below, 120, 15, &mLatencyMs, 0, 1000);
   mDisplayLengthMs = mDisplayLengthMsSlider->GetMax();
}

AudioSyncer::~AudioSyncer()
{
}

void AudioSyncer::Process(double time)
{
   PROFILER(AudioSyncer);

   IAudioReceiver* target = GetTarget();

   SyncBuffers();

   mWriteBuffer.SetNumActiveChannels(1);
   mDelayBuffer.SetNumChannels(1);

   int bufferSize = gBufferSize;

   BufferCopy(mWriteBuffer.GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);
   mBiquadFilter.Filter(mWriteBuffer.GetChannel(0), gBufferSize);
   mDelayBuffer.WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

   if (mPassthrough)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (target)
            Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);

         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
      }
   }

   GetBuffer()->Reset();
}

void AudioSyncer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPassthroughCheckbox->Draw();
   mDisplayLengthMsSlider->Draw();
   mLatencyMsSlider->Draw();

   if (mMono)
   {
      mDelayBuffer.Draw(mBufferX, mBufferY, mBufferW, mBufferH, mDisplayLengthMs * gSampleRateMs, 0);
   }
   else
   {
      for (int ch = 0; ch < mDelayBuffer.NumChannels(); ++ch)
         mDelayBuffer.Draw(mBufferX, mBufferY + mBufferH / mDelayBuffer.NumChannels() * ch, mBufferW, mBufferH / mDelayBuffer.NumChannels(), mDisplayLengthMs * gSampleRateMs, ch);
   }

   ofPushStyle();

   float msPerBeat = TheTransport->MsPerBar() / TheTransport->GetTimeSigTop();
   int numDisplayBeats = int(mDisplayLengthMs / msPerBeat) + 1;
   int beat = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop());
   float beatProgress = TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop() - beat;
   int displayBeat = beat;
   for (int i = 0; i < numDisplayBeats; ++i)
   {
      float msSinceBeat = (i + beatProgress) * msPerBeat - mLatencyMs;
      float x = mBufferX + mBufferW - (msSinceBeat / mDisplayLengthMs) * mBufferW;

      if (displayBeat == 0)
         ofSetColor(255, 255, 0, 70);
      else
         ofSetColor(255, 180, 0, 70);

      ofLine(x, mBufferY, x, mBufferY + mBufferH / 3);
      ofLine(x, mBufferY + mBufferH * 2 / 3, x, mBufferY + mBufferH);

      --displayBeat;
      if (displayBeat < 0)
         displayBeat += TheTransport->GetTimeSigTop();
   }

   ofFill();
   for (int beatSquare = 0; beatSquare < TheTransport->GetTimeSigTop(); ++beatSquare)
   {
      if (beatSquare == 0)
         ofSetColor(255, 255, 0, beatSquare == beat ? 255 : 40);
      else
         ofSetColor(255, 180, 0, beatSquare == beat ? 255 : 40);

      ofRect(150 + beatSquare * 30, 10, 25, 25);
   }

   ofPopStyle();
}

void AudioSyncer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void AudioSyncer::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool AudioSyncer::MouseMoved(float x, float y)
{
   return IDrawableModule::MouseMoved(x, y);
}

void AudioSyncer::GetModuleDimensions(float& width, float& height)
{
   width = mBufferW + 10;
   height = mBufferY + mBufferH + 5;
}

void AudioSyncer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void AudioSyncer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void AudioSyncer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mDelayBuffer.SaveState(out);
}

void AudioSyncer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mDelayBuffer.LoadState(in);
}

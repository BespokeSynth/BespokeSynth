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

const float kBufferX = 5;
const float kBufferY = 60;
const float kBufferW = 800;
const float kBufferH = 200;
const float kBufferLengthMs = 5000;

AudioSyncer::AudioSyncer()
: IAudioProcessor(gBufferSize)
, mWriteBuffer(gBufferSize)
, mDelayBuffer(kBufferLengthMs * gSampleRateMs)
{
   mBiquadFilter.SetFilterType(kFilterType_Highpass);
   mBiquadFilter.SetFilterParams(2000, sqrt(2.0f) / 2);
   mBiquadFilter.UpdateFilterCoeff();
}

void AudioSyncer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mPassthroughCheckbox = new Checkbox(this, "passthrough", 5, 5, &mPassthrough);
   mStaticWaveformCheckbox = new Checkbox(this, "static waveform", mPassthroughCheckbox, kAnchor_Below, &mStaticWaveform);
   mDisplayLengthMsSlider = new FloatSlider(this, "display length", mStaticWaveformCheckbox, kAnchor_Right, 120, 15, &mDisplayLengthMs, .1f, mDelayBuffer.Size() / gSampleRateMs);
   mLatencyMsSlider = new FloatSlider(this, "latencyMs", mStaticWaveformCheckbox, kAnchor_Below, 120, 15, &mLatencyMs, 0, 250);
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

   mDisplayLengthMsSlider->SetShowing(!mStaticWaveform);

   mPassthroughCheckbox->Draw();
   mDisplayLengthMsSlider->Draw();
   mLatencyMsSlider->Draw();
   mStaticWaveformCheckbox->Draw();

   int displaySamples = mDisplayLengthMs * gSampleRateMs;
   if (mStaticWaveform)
   {
      displaySamples = -1; //makes rolling buffer draw statically
      mDisplayLengthMs = kBufferLengthMs;
   }

   if (mMono)
   {
      mDelayBuffer.Draw(kBufferX, kBufferY, kBufferW, kBufferH, displaySamples, 0);
   }
   else
   {
      for (int ch = 0; ch < mDelayBuffer.NumChannels(); ++ch)
         mDelayBuffer.Draw(kBufferX, kBufferY + kBufferH / mDelayBuffer.NumChannels() * ch, kBufferW, kBufferH / mDelayBuffer.NumChannels(), displaySamples, ch);
   }

   ofPushStyle();

   float nowPosX = 0;
   if (mStaticWaveform)
   {
      ofSetColor(0, 255, 0);
      nowPosX = float(mDelayBuffer.GetRawBufferOffset(0)) / mDelayBuffer.Size() * kBufferW + kBufferX;
      ofLine(nowPosX, kBufferY, nowPosX, kBufferY + kBufferH);
   }

   float msPerBeat = TheTransport->MsPerBar() / TheTransport->GetTimeSigTop();
   int numDisplayBeats = int(mDisplayLengthMs / msPerBeat) + 1;
   int beat = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop());
   float beatProgress = TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop() - beat;
   int displayBeat = beat;
   for (int i = 0; i < numDisplayBeats; ++i)
   {
      float msSinceBeat = (i + beatProgress) * msPerBeat - mLatencyMs;
      float x = kBufferX + kBufferW - (msSinceBeat / mDisplayLengthMs) * kBufferW;

      if (mStaticWaveform)
         x = FloatWrap((x - kBufferX) + (nowPosX - kBufferX), kBufferW) + kBufferX;

      float alpha = 70;
      if (msSinceBeat > mDisplayLengthMs * .75f)
         alpha *= ofMap(msSinceBeat, mDisplayLengthMs * .75f, mDisplayLengthMs * .9f, 1, 0, true);

      if (displayBeat == 0)
         ofSetColor(255, 255, 0, alpha);
      else
         ofSetColor(255, 180, 0, alpha);

      ofLine(x, kBufferY, x, kBufferY + kBufferH / 3);
      ofLine(x, kBufferY + kBufferH * 2 / 3, x, kBufferY + kBufferH);

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

      ofRect(250 + beatSquare * 30, 10, 25, 25);
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
   width = kBufferW + 10;
   height = kBufferY + kBufferH + 5;
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

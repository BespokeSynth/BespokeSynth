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

AudioSyncer::BandBuffer::BandBuffer()
: mBuffer(kBufferLengthMs * gSampleRateMs)
{
   mBuffer.SetNumChannels(1);
}

AudioSyncer::AudioSyncer()
: IAudioProcessor(gBufferSize)
{
   BandBuffer* highs = new BandBuffer();
   highs->mFilters.push_back(new BiquadFilter());
   highs->mFilters[0]->SetFilterType(kFilterType_Highpass);
   highs->mFilters[0]->SetFilterParams(2000, sqrt(2.0f) / 2);
   highs->mFilters[0]->UpdateFilterCoeff();
   highs->mColor = ofColor::white;
   highs->mColor.a = 100;
   mBandBuffers.push_back(highs);

   BandBuffer* mids = new BandBuffer();
   mids->mFilters.push_back(new BiquadFilter());
   mids->mFilters[0]->SetFilterType(kFilterType_Highpass);
   mids->mFilters[0]->SetFilterParams(180, sqrt(2.0f) / 2);
   mids->mFilters[0]->UpdateFilterCoeff();
   mids->mFilters.push_back(new BiquadFilter());
   mids->mFilters[1]->SetFilterType(kFilterType_Lowpass);
   mids->mFilters[1]->SetFilterParams(800, sqrt(2.0f) / 2);
   mids->mFilters[1]->UpdateFilterCoeff();
   mids->mColor = ofColor::orange;
   mids->mColor.a = 100;
   mBandBuffers.push_back(mids);

   BandBuffer* lows = new BandBuffer();
   lows->mFilters.push_back(new BiquadFilter());
   lows->mFilters[0]->SetFilterType(kFilterType_Highpass);
   lows->mFilters[0]->SetFilterParams(25, sqrt(2.0f) / 2);
   lows->mFilters[0]->UpdateFilterCoeff();
   lows->mFilters.push_back(new BiquadFilter());
   lows->mFilters[1]->SetFilterType(kFilterType_Lowpass);
   lows->mFilters[1]->SetFilterParams(110, sqrt(2.0f) / 2);
   lows->mFilters[01]->UpdateFilterCoeff();
   lows->mColor = ofColor::blue;
   lows->mColor.a = 100;
   mBandBuffers.push_back(lows);
}

void AudioSyncer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mPassthroughCheckbox = new Checkbox(this, "passthrough", 5, 5, &mPassthrough);
   mStaticWaveformCheckbox = new Checkbox(this, "static waveform", mPassthroughCheckbox, kAnchor_Below, &mStaticWaveform);
   mDisplayLengthMsSlider = new FloatSlider(this, "display length", mStaticWaveformCheckbox, kAnchor_Right, 120, 15, &mDisplayLengthMs, .1f, mBandBuffers[0]->mBuffer.Size() / gSampleRateMs);
   mLatencyMsSlider = new FloatSlider(this, "latencyMs", mStaticWaveformCheckbox, kAnchor_Below, 120, 15, &mLatencyMs, 0, 250);
   mDisplayLengthMs = mDisplayLengthMsSlider->GetMax();
}

AudioSyncer::~AudioSyncer()
{
   for (auto* bandBuffer : mBandBuffers)
   {
      for (auto* filter : bandBuffer->mFilters)
         delete filter;
      delete bandBuffer;
   }
}

void AudioSyncer::Process(double time)
{
   PROFILER(AudioSyncer);

   IAudioReceiver* target = GetTarget();

   SyncBuffers();

   int bufferSize = gBufferSize;

   for (auto* bandBuffer : mBandBuffers)
   {
      BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(0), bufferSize);
      for (auto* filter : bandBuffer->mFilters)
         filter->Filter(gWorkBuffer, gBufferSize);
      bandBuffer->mBuffer.WriteChunk(gWorkBuffer, bufferSize, 0);
   }

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

   for (auto& bandBuffer : mBandBuffers)
      bandBuffer->mBuffer.Draw(kBufferX, kBufferY, kBufferW, kBufferH, displaySamples, 0, 0, bandBuffer->mColor, false);

   ofPushStyle();

   float nowPosX = 0;
   if (mStaticWaveform)
   {
      ofSetColor(0, 255, 0);
      nowPosX = float(mBandBuffers[0]->mBuffer.GetRawBufferOffset(0)) / mBandBuffers[0]->mBuffer.Size() * kBufferW + kBufferX;
      ofLine(nowPosX, kBufferY, nowPosX, kBufferY + kBufferH);
   }

   ofFill();
   int subdivide = 2;
   int beatsPerBar = TheTransport->GetTimeSigTop() * subdivide;
   float msPerBeat = TheTransport->MsPerBar() / beatsPerBar;
   int numDisplayBeats = int(mDisplayLengthMs / msPerBeat) + 1;
   int beat = int(TheTransport->GetMeasurePos(gTime) * beatsPerBar);
   float beatProgress = TheTransport->GetMeasurePos(gTime) * beatsPerBar - beat;
   int displayBeat = beat;
   for (int i = 0; i < numDisplayBeats; ++i)
   {
      float msSinceBeat = (i + beatProgress) * msPerBeat - mLatencyMs;
      float x = kBufferX + kBufferW - (msSinceBeat / mDisplayLengthMs) * kBufferW;

      if (mStaticWaveform)
         x = FloatWrap((x - kBufferX) + (nowPosX - kBufferX), kBufferW) + kBufferX;

      float alpha = 255;
      if (msSinceBeat > mDisplayLengthMs * .75f)
         alpha *= ofMap(msSinceBeat, mDisplayLengthMs * .75f, mDisplayLengthMs * .9f, 1, 0, true);

      if (displayBeat == 0)
         ofSetColor(255, 255, 0, alpha);
      else if (displayBeat % subdivide != 0)
         ofSetColor(255, 255, 255, alpha);
      else
         ofSetColor(255, 180, 0, alpha);

      if (displayBeat % subdivide == 0)
      {
         ofRectangle(x - 2, kBufferY - 4, 4, 4);
         ofLine(x, kBufferY, x, kBufferY + kBufferH / 3);
         ofLine(x, kBufferY + kBufferH * 2 / 3, x, kBufferY + kBufferH);
         ofRectangle(x - 2, kBufferY + kBufferH, 4, 4);
      }
      else
      {
         ofLine(x, kBufferY + kBufferH * .1f, x, kBufferY + kBufferH * .25f);
         ofLine(x, kBufferY + kBufferH * .75f, x, kBufferY + kBufferH * .9f);
      }

      --displayBeat;
      if (displayBeat < 0)
         displayBeat += beatsPerBar;
   }

   ofFill();
   beat = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop());
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
}

void AudioSyncer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev <= 0)
   {
      RollingBuffer temp(kBufferLengthMs * gSampleRateMs);
      temp.LoadState(in);
   }
}

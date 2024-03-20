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
//  Vocoder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/17/13.
//
//

#include "Vocoder.h"
#include "ModularSynth.h"
#include "Profiler.h"

Vocoder::Vocoder()
: IAudioProcessor(gBufferSize)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[VOCODER_WINDOW_SIZE];
   for (int i = 0; i < VOCODER_WINDOW_SIZE; ++i)
      mWindower[i] = -.5 * cos(FTWO_PI * i / VOCODER_WINDOW_SIZE) + .5;

   mCarrierInputBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mCarrierInputBuffer, GetBuffer()->BufferSize());

   AddChild(&mGate);
   mGate.SetPosition(110, 20);
   mGate.SetEnabled(false);
}

void Vocoder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mInputSlider = new FloatSlider(this, "input", 5, 29, 100, 15, &mInputPreamp, 0, 2);
   mCarrierSlider = new FloatSlider(this, "carrier", 5, 47, 100, 15, &mCarrierPreamp, 0, 2);
   mVolumeSlider = new FloatSlider(this, "volume", 5, 65, 100, 15, &mVolume, 0, 2);
   mDryWetSlider = new FloatSlider(this, "dry/wet", 5, 83, 100, 15, &mDryWet, 0, 1);
   mFricativeSlider = new FloatSlider(this, "fric thresh", 5, 101, 100, 15, &mFricativeThresh, 0, 1);
   mWhisperSlider = new FloatSlider(this, "whisper", 5, 119, 100, 15, &mWhisper, 0, 1);
   mPhaseOffsetSlider = new FloatSlider(this, "phase off", 5, 137, 100, 15, &mPhaseOffset, 0, FTWO_PI);
   mCutSlider = new IntSlider(this, "cut", 5, 155, 100, 15, &mCut, 0, 100);

   mGate.CreateUIControls();
}

Vocoder::~Vocoder()
{
   delete[] mWindower;
   delete[] mCarrierInputBuffer;
}

void Vocoder::SetCarrierBuffer(float* carrier, int bufferSize)
{
   assert(bufferSize == GetBuffer()->BufferSize());
   BufferCopy(mCarrierInputBuffer, carrier, bufferSize);
   mCarrierDataSet = true;
}

void Vocoder::Process(double time)
{
   PROFILER(Vocoder);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   ComputeSliders(0);

   float inputPreampSq = mInputPreamp * mInputPreamp;
   float carrierPreampSq = mCarrierPreamp * mCarrierPreamp;
   float volSq = mVolume * mVolume;

   int bufferSize = GetBuffer()->BufferSize();

   int zerox = 0; //count up zero crossings
   bool positive = true;
   for (int i = 0; i < bufferSize; ++i)
   {
      if ((GetBuffer()->GetChannel(0)[i] < 0 && positive) ||
          (GetBuffer()->GetChannel(0)[i] > 0 && !positive))
      {
         ++zerox;
         positive = !positive;
      }
   }
   bool fricative = zerox > (bufferSize * mFricativeThresh); //if a % of the samples are zero crossings, this is a fricative
   if (fricative)
      mFricDetected = true; //draw that we detected a fricative

   mGate.ProcessAudio(time, GetBuffer());

   mRollingInputBuffer.WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

   //copy rolling input buffer into working buffer and window it
   mRollingInputBuffer.ReadChunk(mFFTData.mTimeDomain, VOCODER_WINDOW_SIZE, 0, 0);
   Mult(mFFTData.mTimeDomain, mWindower, VOCODER_WINDOW_SIZE);
   Mult(mFFTData.mTimeDomain, inputPreampSq, VOCODER_WINDOW_SIZE);

   mFFT.Forward(mFFTData.mTimeDomain,
                mFFTData.mRealValues,
                mFFTData.mImaginaryValues);

   if (!fricative)
   {
      mRollingCarrierBuffer.WriteChunk(mCarrierInputBuffer, bufferSize, 0);
   }
   else
   {
      //use noise as carrier signal if it's a fricative
      //but make the noise the same-ish volume as input carrier
      for (int i = 0; i < bufferSize; ++i)
         mRollingCarrierBuffer.Write(mCarrierInputBuffer[gRandom() % bufferSize] * 2, 0);
   }

   //copy rolling carrier buffer into working buffer and window it
   mRollingCarrierBuffer.ReadChunk(mCarrierFFTData.mTimeDomain, VOCODER_WINDOW_SIZE, 0, 0);
   Mult(mCarrierFFTData.mTimeDomain, mWindower, VOCODER_WINDOW_SIZE);
   Mult(mCarrierFFTData.mTimeDomain, carrierPreampSq, VOCODER_WINDOW_SIZE);

   mFFT.Forward(mCarrierFFTData.mTimeDomain,
                mCarrierFFTData.mRealValues,
                mCarrierFFTData.mImaginaryValues);

   for (int i = 0; i < FFT_FREQDOMAIN_SIZE; ++i)
   {
      float real = mFFTData.mRealValues[i];
      float imag = mFFTData.mImaginaryValues[i];

      //cartesian to polar
      float amp = 2. * sqrtf(real * real + imag * imag);
      //float phase = atan2(imag,real);

      float carrierReal = mCarrierFFTData.mRealValues[i];
      float carrierImag = mCarrierFFTData.mImaginaryValues[i];

      //cartesian to polar
      float carrierAmp = 2. * sqrtf(carrierReal * carrierReal + carrierImag * carrierImag);
      float carrierPhase = atan2(carrierImag, carrierReal);

      amp *= carrierAmp;
      float phase = carrierPhase;

      phase += ofRandom(mWhisper * FTWO_PI);
      mPhaseOffsetSlider->Compute();
      phase = FloatWrap(phase + mPhaseOffset, FTWO_PI);

      if (i < mCut) //cut out superbass
         amp = 0;

      //polar to cartesian
      real = amp * cos(phase);
      imag = amp * sin(phase);

      mFFTData.mRealValues[i] = real;
      mFFTData.mImaginaryValues[i] = imag;
   }

   mFFT.Inverse(mFFTData.mRealValues,
                mFFTData.mImaginaryValues,
                mFFTData.mTimeDomain);

   for (int i = 0; i < bufferSize; ++i)
      mRollingOutputBuffer.Write(0, 0);

   //copy rolling input buffer into working buffer and window it
   for (int i = 0; i < VOCODER_WINDOW_SIZE; ++i)
      mRollingOutputBuffer.Accum(VOCODER_WINDOW_SIZE - i - 1, mFFTData.mTimeDomain[i] * mWindower[i] * .0001f, 0);

   Mult(GetBuffer()->GetChannel(0), (1 - mDryWet) * inputPreampSq, GetBuffer()->BufferSize());

   for (int i = 0; i < bufferSize; ++i)
      GetBuffer()->GetChannel(0)[i] += mRollingOutputBuffer.GetSample(VOCODER_WINDOW_SIZE - i - 1, 0) * volSq * mDryWet;

   Add(target->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

   GetBuffer()->Reset();
}

void Vocoder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!mCarrierDataSet)
   {
      ofPushStyle();
      ofSetColor(255, 0, 0);
      DrawTextNormal("no vocodercarrier!", 5, 15);
      ofPopStyle();
   }

   mInputSlider->Draw();
   mCarrierSlider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   mFricativeSlider->Draw();
   mWhisperSlider->Draw();
   mPhaseOffsetSlider->Draw();
   mCutSlider->Draw();

   if (mFricDetected)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0, gModuleDrawAlpha * .4f);
      ofRect(5, 101, 100, 14);
      ofPopStyle();
      mFricDetected = false;
   }

   mGate.Draw();
}

void Vocoder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mGate.SetEnabled(mEnabled);
   }
}

void Vocoder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Vocoder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

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
//  PadSynthVoice.cpp
//  modularSynth
//
//  Created by Andrius Merkys on 4/8/25.
//
//

// Contains PADsynth code generously donated to public domain by
// Nasca Octavian Paul, accessible at
// https://zynaddsubfx.sourceforge.io/doc/PADsynth/PADsynth.htm

#include "PadSynthVoice.h"
#include "PadSynth.h"
#include "EnvOscillator.h"
#include "FFT.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"
#include "PolyphonyMgr.h"
#include "SingleOscillatorVoice.h"

#include "juce_core/juce_core.h"

PadSynthVoice::PadSynthVoice(IDrawableModule* owner)
: mOwner(owner)
{
}

PadSynthVoice::~PadSynthVoice()
{
   delete mFFT;
}

bool PadSynthVoice::IsDone(double time)
{
   return mAdsr.IsDone(time);
}

bool PadSynthVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(PadSynthVoice);

   if (IsDone(time))
      return false;

   int bufferSize = out->BufferSize();
   int channels = out->NumActiveChannels();
   double sampleIncrementMs = gInvSampleRateMs;
   double sampleRate = gSampleRate;
   ChannelBuffer* destBuffer = out;

   float freq;
   float pitch;

   if (mVoiceParams->mLiteCPUMode)
      DoParameterUpdate(0, pitch, freq);

   // Setting undersample to a number greater than 1 increases the
   // internal buffer size to allow for longer wavetables
   int undersample = 1;
   if (mVoiceParams->mUndersample > 0)
      undersample *= 2 << (mVoiceParams->mUndersample - 1);
   int extendedBufferSize = bufferSize * undersample;

   float* freq_amp = (float*)calloc(extendedBufferSize / 2, sizeof(float));
   for (int i = 0; i < extendedBufferSize / 2; i++)
      freq_amp[i] = 0.0;

   int harmonics = mVoiceParams->mHarmonics;
   if (mVoiceParams->mHarmonicsRelative && harmonics != 0)
      if (harmonics > 0)
         harmonics = 440.0 / freq * harmonics;
      else
         harmonics = freq / 440.0 * harmonics;

   for (int nh = 1; nh <= abs(harmonics); nh++) // for each harmonic
   {
      float relf = pow(nh, abs(harmonics) / harmonics) * (1.0 + nh * mVoiceParams->mSpread);
      float bw_Hz = (pow(2.0, mVoiceParams->mBandwidth / 1200.0) - 1.0) * freq * pow(relf, mVoiceParams->mBandwidthScale); // bandwidth of the current harmonic measured in Hz
      float bwi = bw_Hz / (2.0 * sampleRate);
      float fi = freq * relf / sampleRate;

      float A = 1.0;
      if (mVoiceParams->mAmplitudeType == kAmplitudeTypeStep)
         A = A / nh * (2 - (nh % 2));
      else if (mVoiceParams->mAmplitudeType == kAmplitudeTypeSqrt)
         A = A / pow((float)nh, 0.5);

      for (int i = 0; i < extendedBufferSize / 2; i++)
      {
         float hprofile = 0.0;
         float x = ((i / (float)extendedBufferSize) - fi) / bwi;
         x = x * x;
         if (x <= 14.71280603) // this avoids computing the e^(-x^2) where it's results are very close to zero
            hprofile = exp(-x) / bwi;
         freq_amp[i] += hprofile * A;
      }
   }

   float* freq_real = (float*)calloc(extendedBufferSize / 2, sizeof(float));
   float* freq_imag = (float*)calloc(extendedBufferSize / 2, sizeof(float));

   for (int i = 0; i < extendedBufferSize / 2; i++)
   {
      // Use random phase
      float phase = ((abs(DeterministicRandom((int)pitch, i)) % 10000) / 10000.0f) * 2.0 * PI;
      freq_real[i] = freq_amp[i] * cos(phase);
      freq_imag[i] = freq_amp[i] * sin(phase);
   }

   free(freq_amp);

   float* sample = (float*)calloc(extendedBufferSize, sizeof(float));
   mFFT = new ::FFT(extendedBufferSize);
   mFFT->Inverse(freq_real, freq_imag, sample);

   free(freq_real);
   free(freq_imag);

   // Normalize the sound to 1/sqrt(2)
   float max = 0.0;
   for (int i = 0; i < extendedBufferSize; i++)
      if (fabsf(sample[i]) > max)
         max = fabsf(sample[i]);
   if (max < 1e-5)
      max = 1e-5;
   for (int i = 0; i < extendedBufferSize; i++)
      sample[i] /= max * 1.4142;

   for (int pos = 0; pos < bufferSize; ++pos)
   {
      if (!mVoiceParams->mLiteCPUMode)
         DoParameterUpdate(pos, pitch, freq);

      if (channels == 1)
      {
         destBuffer->GetChannel(0)[pos] += sample[mSample * bufferSize + pos] * mAdsr.Value(time);
      }
      else
      {
         int channel_offset = extendedBufferSize * mVoiceParams->mChannelOffset;
         destBuffer->GetChannel(0)[pos] += sample[mSample * bufferSize + pos] * GetLeftPanGain(GetPan()) * mAdsr.Value(time);
         destBuffer->GetChannel(1)[pos] += sample[(mSample * bufferSize + pos + channel_offset) % extendedBufferSize] * GetRightPanGain(GetPan()) * mAdsr.Value(time);
      }

      time += sampleIncrementMs;
   }

   free(sample);

   mSample = (mSample + 1) % undersample;

   return true;
}

void PadSynthVoice::DoParameterUpdate(int samplesIn, float& pitch, float& freq)
{
   if (mOwner)
      mOwner->ComputeSliders(samplesIn);

   pitch = GetPitch(samplesIn);
   freq = TheScale->PitchToFreq(pitch);
}

void PadSynthVoice::Start(double time, float target)
{
   float volume = ofLerp((1 - mVoiceParams->mVelToVolume), 1, target * target);
   float adsrCurve = SingleOscillatorVoice::GetADSRCurve(target, mVoiceParams->mVelToEnvelope);
   mAdsr.Start(time, volume, mVoiceParams->mAdsr, 1, adsrCurve);
}

void PadSynthVoice::Stop(double time)
{
   mAdsr.Stop(time);
}

void PadSynthVoice::ClearVoice()
{
   mAdsr.Clear();
}

void PadSynthVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<PadSynthVoiceParams*>(params);
}

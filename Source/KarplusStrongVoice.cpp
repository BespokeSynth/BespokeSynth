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
//  KarplusStrongVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "KarplusStrongVoice.h"
#include "KarplusStrong.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"
#include "PolyphonyMgr.h"
#include "SingleOscillatorVoice.h"

#include "juce_core/juce_core.h"

KarplusStrongVoice::KarplusStrongVoice(IDrawableModule* owner)
: mBuffer(gSampleRate)
, mOwner(owner)
{
   mOsc.Start(0, 1);
   mEnv.SetNumStages(2);
   mEnv.GetHasSustainStage() = false;
   mEnv.GetStageData(0).target = 1;
   mEnv.GetStageData(0).time = 3;
   mEnv.GetStageData(1).target = 0;
   mEnv.GetStageData(1).time = 3;
   ClearVoice();

   mKarplusStrongModule = dynamic_cast<KarplusStrong*>(mOwner);
}

KarplusStrongVoice::~KarplusStrongVoice()
{
}

bool KarplusStrongVoice::IsDone(double time)
{
   return !mActive || mMuteRamp.Value(time) == 0;
}

bool KarplusStrongVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(KarplusStrongVoice);

   if (IsDone(time))
      return false;

   int bufferSize = out->BufferSize();
   int channels = out->NumActiveChannels();
   double sampleIncrementMs = gInvSampleRateMs;
   double sampleRate = gSampleRate;
   ChannelBuffer* destBuffer = out;

   if (oversampling != 1)
   {
      gMidiVoiceWorkChannelBuffer.SetNumActiveChannels(channels);
      destBuffer = &gMidiVoiceWorkChannelBuffer;
      gMidiVoiceWorkChannelBuffer.Clear();
      bufferSize *= oversampling;
      sampleIncrementMs /= oversampling;
      sampleRate *= oversampling;
   }

   float freq;
   float filterRate;
   float filterLerp;
   float pitch;
   float oscPhaseInc;

   if (mVoiceParams->mLiteCPUMode)
      DoParameterUpdate(0, oversampling, pitch, freq, filterRate, filterLerp, oscPhaseInc);

   for (int pos = 0; pos < bufferSize; ++pos)
   {
      if (!mVoiceParams->mLiteCPUMode)
         DoParameterUpdate(pos / oversampling, oversampling, pitch, freq, filterRate, filterLerp, oscPhaseInc);

      if (mVoiceParams->mSourceType == kSourceTypeSaw)
         mOsc.SetType(kOsc_Saw);
      else
         mOsc.SetType(kOsc_Sin);
      mOscPhase += oscPhaseInc;
      float sample = 0;
      float oscSample = mOsc.Audio(time, mOscPhase);
      float noiseSample = RandomSample();
      float pitchBlend = ofClamp((pitch - 40) / 60.0f, 0, 1);
      pitchBlend *= pitchBlend;
      if (mVoiceParams->mSourceType == kSourceTypeSin || mVoiceParams->mSourceType == kSourceTypeSaw)
         sample = oscSample;
      else if (mVoiceParams->mSourceType == kSourceTypeNoise)
         sample = noiseSample;
      else if (mVoiceParams->mSourceType == kSourceTypeMix)
         sample = noiseSample * pitchBlend + oscSample * (1 - pitchBlend);
      else if (mVoiceParams->mSourceType == kSourceTypeInput || mVoiceParams->mSourceType == kSourceTypeInputNoEnvelope)
         sample = mKarplusStrongModule->GetBuffer()->GetChannel(0)[pos / oversampling];

      if (mVoiceParams->mSourceType != kSourceTypeInputNoEnvelope)
         sample *= mEnv.Value(time) + mVoiceParams->mExcitation;

      float samplesAgo = sampleRate / freq;
      AssertIfDenormal(samplesAgo);
      float feedbackSample = 0;
      if (samplesAgo < mBuffer.Size())
      {
         //interpolated delay
         int delay_pos = int(samplesAgo);
         int posNext = int(samplesAgo) + 1;
         if (delay_pos < mBuffer.Size())
         {
            float delay_sample = delay_pos < 0 ? 0 : mBuffer.GetSample(delay_pos, 0);
            float nextSample = posNext >= mBuffer.Size() ? 0 : mBuffer.GetSample(posNext, 0);
            float a = samplesAgo - delay_pos;
            feedbackSample = (1 - a) * delay_sample + a * nextSample; //interpolate
            JUCE_UNDENORMALISE(feedbackSample);
         }
      }
      mFilteredSample = ofLerp(feedbackSample, mFilteredSample, filterLerp);
      JUCE_UNDENORMALISE(mFilteredSample);
      //sample += mFeedbackRamp.Value(time) * mFilterSample;
      float feedback = mFilteredSample * sqrtf(mVoiceParams->mFeedback + GetPressure(pos) * .02f) * mMuteRamp.Value(time);
      if (mVoiceParams->mInvert)
         feedback *= -1;

      float sampleForFeedbackBuffer = sample + feedback;
      float outputSample;
      if (mVoiceParams->mSourceType == kSourceTypeInputNoEnvelope)
         outputSample = feedback; //don't include dry input in the output
      else
         outputSample = sampleForFeedbackBuffer;
      JUCE_UNDENORMALISE(sample);

      mBuffer.Write(sampleForFeedbackBuffer, 0);

      if (channels == 1)
      {
         destBuffer->GetChannel(0)[pos] += outputSample;
      }
      else
      {
         destBuffer->GetChannel(0)[pos] += outputSample * GetLeftPanGain(GetPan());
         destBuffer->GetChannel(1)[pos] += outputSample * GetRightPanGain(GetPan());
      }

      time += sampleIncrementMs;
   }

   if (oversampling != 1)
   {
      //assume power-of-two
      while (oversampling > 1)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            for (int ch = 0; ch < channels; ++ch)
               destBuffer->GetChannel(ch)[i] = (destBuffer->GetChannel(ch)[i * 2] + destBuffer->GetChannel(ch)[i * 2 + 1]) / 2;
         }
         oversampling /= 2;
         bufferSize /= 2;
      }

      for (int ch = 0; ch < channels; ++ch)
         Add(out->GetChannel(ch), destBuffer->GetChannel(ch), bufferSize);
   }

   return true;
}

void KarplusStrongVoice::DoParameterUpdate(int samplesIn,
                                           int oversampling,
                                           float& pitch,
                                           float& freq,
                                           float& filterRate,
                                           float& filterLerp,
                                           float& oscPhaseInc)
{
   if (mOwner)
      mOwner->ComputeSliders(samplesIn);

   pitch = GetPitch(samplesIn);
   if (mVoiceParams->mInvert)
      pitch += 12; //inverting the pitch gives an octave down sound by halving the resonating frequency, so correct for that

   freq = TheScale->PitchToFreq(pitch);
   filterRate = mVoiceParams->mFilter * pow(freq / 300, exp2(mVoiceParams->mPitchTone)) * (1 + GetModWheel(samplesIn));
   filterLerp = ofClamp(exp2(-filterRate / oversampling), 0, 1);

   oscPhaseInc = GetPhaseInc(mVoiceParams->mExciterFreq) / oversampling;
}

void KarplusStrongVoice::Start(double time, float target)
{
   float volume = ofLerp((1 - mVoiceParams->mVelToVolume), 1, target * target);
   float envScale = SingleOscillatorVoice::GetADSRScale(target, -mVoiceParams->mVelToEnvelope);

   mOscPhase = FPI / 2; //magic number that seems to keep things DC centered ok
   mEnv.Clear();
   mEnv.GetStageData(0).time = mVoiceParams->mExciterAttack * envScale;
   mEnv.GetStageData(1).time = mVoiceParams->mExciterDecay;
   mEnv.Start(time, volume);
   mEnv.SetMaxSustain(10);
   mMuteRamp.SetValue(1);
   mActive = true;
}

void KarplusStrongVoice::Stop(double time)
{
   mMuteRamp.Start(time, 0, time + 400);
}

void KarplusStrongVoice::ClearVoice()
{
   mBuffer.ClearBuffer();
   mFilteredSample = 0;
   mActive = false;
}

void KarplusStrongVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<KarplusStrongVoiceParams*>(params);
}

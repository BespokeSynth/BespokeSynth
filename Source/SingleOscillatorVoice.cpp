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
//  SingleOscillatorVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#include "SingleOscillatorVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"

SingleOscillatorVoice::SingleOscillatorVoice(IDrawableModule* owner)
: mOwner(owner)
{
}

SingleOscillatorVoice::~SingleOscillatorVoice()
{
}

bool SingleOscillatorVoice::IsDone(double time)
{
   return mAdsr.IsDone(time);
}

bool SingleOscillatorVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(SingleOscillatorVoice);

   if (IsDone(time))
      return false;

   for (int u = 0; u < mVoiceParams->mUnison && u < kMaxUnison; ++u)
      mOscData[u].mOsc.SetType(mVoiceParams->mOscType);

   bool mono = (out->NumActiveChannels() == 1);

   float pitch;
   float freq;
   float vol;
   float syncPhaseInc;

   if (mVoiceParams->mLiteCPUMode)
      DoParameterUpdate(0, pitch, freq, vol, syncPhaseInc);

   for (int pos = 0; pos < out->BufferSize(); ++pos)
   {
      if (!mVoiceParams->mLiteCPUMode)
         DoParameterUpdate(pos, pitch, freq, vol, syncPhaseInc);

      float adsrVal = mAdsr.Value(time);

      float summedLeft = 0;
      float summedRight = 0;
      for (int u = 0; u < mVoiceParams->mUnison && u < kMaxUnison; ++u)
      {
         mOscData[u].mOsc.SetPulseWidth(mVoiceParams->mPulseWidth);
         mOscData[u].mOsc.SetShuffle(mVoiceParams->mShuffle);
         mOscData[u].mOsc.SetSoften(mVoiceParams->mSoften);

         {
            //PROFILER(SingleOscillatorVoice_UpdatePhase);
            mOscData[u].mPhase += mOscData[u].mCurrentPhaseInc;
            if (std::isinf(mOscData[u].mPhase))
            {
               ofLog() << "Infinite phase. phaseInc:" + ofToString(mOscData[u].mCurrentPhaseInc) + " detune:" + ofToString(mVoiceParams->mDetune) + " freq:" + ofToString(freq) + " pitch:" + ofToString(pitch) + " getpitch:" + ofToString(GetPitch(pos));
               // Reset to 0 because letting this propagate causes NaN's
               mOscData[u].mPhase = 0;
               mOscData[u].mCurrentPhaseInc = 0;
            }
            else
            {
               while (mOscData[u].mPhase > FTWO_PI * 2)
               {
                  mOscData[u].mPhase -= FTWO_PI * 2;
                  mOscData[u].mSyncPhase = 0;
               }
            }
            mOscData[u].mSyncPhase += syncPhaseInc;
         }
         if (std::isinf(mOscData[u].mSyncPhase))
         {
            // Reset to 0 because letting this propagate causes NaN's
            mOscData[u].mSyncPhase = 0;
            syncPhaseInc = 0;
         }

         float sample;

         {
            //PROFILER(SingleOscillatorVoice_GetOscValue);
            if (mVoiceParams->mSyncMode != Oscillator::SyncMode::None)
               sample = mOscData[u].mOsc.Value(mOscData[u].mSyncPhase) * adsrVal * vol;
            else
               sample = mOscData[u].mOsc.Value(mOscData[u].mPhase + mVoiceParams->mPhaseOffset * (1 + (float(u) / mVoiceParams->mUnison))) * adsrVal * vol;
         }

         if (u >= 2)
            sample *= 1 - (mOscData[u].mDetuneFactor * .5f);

         if (mono)
         {
            summedLeft += sample;
         }
         else
         {
            //PROFILER(SingleOscillatorVoice_pan);
            float unisonPan;
            if (mVoiceParams->mUnison == 1)
               unisonPan = 0;
            else if (u == 0)
               unisonPan = -1;
            else if (u == 1)
               unisonPan = 1;
            else
               unisonPan = mOscData[u].mDetuneFactor;
            float pan = GetPan() + unisonPan * mVoiceParams->mUnisonWidth;
            summedLeft += sample * GetLeftPanGain(pan);
            summedRight += sample * GetRightPanGain(pan);
         }
      }

      if (mUseFilter)
      {
         //PROFILER(SingleOscillatorVoice_filter);
         float f = ofLerp(mVoiceParams->mFilterCutoffMin, mVoiceParams->mFilterCutoffMax, mFilterAdsr.Value(time)) * (1 - GetModWheel(pos) * .9f);
         float q = mVoiceParams->mFilterQ;
         if (f != mFilterLeft.mF || q != mFilterLeft.mQ)
            mFilterLeft.SetFilterParams(f, q);
         summedLeft = mFilterLeft.Filter(summedLeft);
         if (!mono)
         {
            mFilterRight.CopyCoeffFrom(mFilterLeft);
            summedRight = mFilterRight.Filter(summedRight);
         }
      }

      {
         //PROFILER(SingleOscillatorVoice_output);
         if (mono)
         {
            out->GetChannel(0)[pos] += summedLeft;
         }
         else
         {
            out->GetChannel(0)[pos] += summedLeft;
            out->GetChannel(1)[pos] += summedRight;
         }
      }
      time += gInvSampleRateMs;
   }

   return true;
}

void SingleOscillatorVoice::DoParameterUpdate(int samplesIn,
                                              float& pitch,
                                              float& freq,
                                              float& vol,
                                              float& syncPhaseInc)
{
   if (mOwner)
      mOwner->ComputeSliders(samplesIn);

   pitch = GetPitch(samplesIn);
   freq = TheScale->PitchToFreq(pitch) * mVoiceParams->mMult;
   vol = mVoiceParams->mVol * .4f / mVoiceParams->mUnison;
   if (mVoiceParams->mSyncMode == Oscillator::SyncMode::Frequency)
      syncPhaseInc = GetPhaseInc(mVoiceParams->mSyncFreq);
   else if (mVoiceParams->mSyncMode == Oscillator::SyncMode::Ratio)
      syncPhaseInc = GetPhaseInc(freq * mVoiceParams->mSyncRatio);
   else
      syncPhaseInc = 0;

   for (int u = 0; u < mVoiceParams->mUnison && u < kMaxUnison; ++u)
   {
      float detune = exp2(mVoiceParams->mDetune * mOscData[u].mDetuneFactor * (1 - GetPressure(samplesIn)));
      mOscData[u].mCurrentPhaseInc = GetPhaseInc(freq * detune);
   }
}

//static
float SingleOscillatorVoice::GetADSRScale(float velocity, float velToEnvelope)
{
   if (velToEnvelope >= 0)
      return ofLerp(ofClamp(1 - velToEnvelope, 0, 1), 1, velocity);
   return ofClamp(ofLerp(1, 1 + velToEnvelope, velocity), 0.001f, 1);
}

//static
float SingleOscillatorVoice::GetADSRCurve(float velocity, float velToEnvelope)
{
   if (velToEnvelope < -1)
      return -(velToEnvelope + 1) * 0.25f;
   if (velToEnvelope > 1)
      return -(velToEnvelope - 1) * 0.25f;
   return 0;
}

void SingleOscillatorVoice::Start(double time, float target)
{
   if (mVoiceParams->mVelToVolume > 1)
      target = pow(target, mVoiceParams->mVelToVolume);
   float volume = ofLerp(MAX(0, 1 - mVoiceParams->mVelToVolume), MAX(1, mVoiceParams->mVelToVolume), target);
   float cutoffScale = 1 + MAX(0, mVoiceParams->mVelToEnvelope - 1);
   float adsrTimeScale = GetADSRScale(target, mVoiceParams->mVelToEnvelope);
   float adsrCurve = GetADSRCurve(target, mVoiceParams->mVelToEnvelope);
   mAdsr.Start(time, volume, mVoiceParams->mAdsr, 1, adsrCurve);

   if (mVoiceParams->mFilterCutoffMax != SINGLEOSCILLATOR_NO_CUTOFF)
   {
      mUseFilter = true;
      mFilterLeft.SetFilterType(kFilterType_Lowpass);
      mFilterRight.SetFilterType(kFilterType_Lowpass);
      mFilterAdsr.Start(time, cutoffScale, mVoiceParams->mFilterAdsr, adsrTimeScale, adsrCurve);
   }
   else
   {
      mUseFilter = false;
   }
}

void SingleOscillatorVoice::Stop(double time)
{
   mAdsr.Stop(time);
   mFilterAdsr.Stop(time);
}

void SingleOscillatorVoice::ClearVoice()
{
   mAdsr.Clear();
   mFilterAdsr.Clear();
   for (int u = 0; u < kMaxUnison; ++u)
   {
      mOscData[u].mPhase = 0;
      mOscData[u].mSyncPhase = 0;
   }

   //set this up so it's different with each fresh voice, but doesn't reset when voice is retriggered
   mOscData[0].mDetuneFactor = 1;
   mOscData[1].mDetuneFactor = 0;
   for (int u = 2; u < kMaxUnison; ++u)
      mOscData[u].mDetuneFactor = ofRandom(-1, 1);
}

void SingleOscillatorVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<OscillatorVoiceParams*>(params);
}

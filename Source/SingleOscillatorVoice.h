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
//  SingleOscillatorVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#pragma once

#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"
#include "LFO.h"
#include "BiquadFilter.h"

#define SINGLEOSCILLATOR_NO_CUTOFF 10000

class OscillatorVoiceParams : public IVoiceParams
{
public:
   ::ADSR mAdsr{ 10, 0, 1, 10 };
   float mVol{ .25 };
   float mPulseWidth{ .5 };
   Oscillator::SyncMode mSyncMode{ Oscillator::SyncMode::None };
   float mSyncFreq{ 200 };
   float mSyncRatio{ 1 };
   float mMult{ 1 };
   OscillatorType mOscType{ OscillatorType::kOsc_Square };
   float mDetune{ 0 };
   float mShuffle{ 0 };
   float mPhaseOffset{ 0 };
   int mUnison{ 1 };
   float mUnisonWidth{ 0 };
   float mSoften{ 0 };

   float mFilterCutoffMax{ SINGLEOSCILLATOR_NO_CUTOFF };
   float mFilterCutoffMin{ 10 };
   float mFilterQ{ float(sqrt(2) / 2) };
   ::ADSR mFilterAdsr{ 1, 0, 1, 1000 };

   float mVelToVolume{ 1.0 };
   float mVelToEnvelope{ 0 };

   bool mLiteCPUMode{ false };
};

class SingleOscillatorVoice : public IMidiVoice
{
public:
   SingleOscillatorVoice(IDrawableModule* owner = nullptr);
   ~SingleOscillatorVoice();

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out, int oversampling) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;

   static float GetADSRScale(float velocity, float velToEnvelope);
   static float GetADSRCurve(float velocity, float velToEnvelope);

   static const int kMaxUnison = 8;

private:
   void DoParameterUpdate(int samplesIn,
                          float& pitch,
                          float& freq,
                          float& vol,
                          float& syncPhaseInc);

   struct OscData
   {
      float mPhase{ 0 };
      float mSyncPhase{ 0 };
      Oscillator mOsc{ kOsc_Square };
      float mDetuneFactor{ 0 };
      float mCurrentPhaseInc{ 0 };
   };
   OscData mOscData[kMaxUnison];
   ::ADSR mAdsr;
   OscillatorVoiceParams* mVoiceParams{ nullptr };

   ::ADSR mFilterAdsr;
   BiquadFilter mFilterLeft;
   BiquadFilter mFilterRight;
   bool mUseFilter{ false };

   IDrawableModule* mOwner;
};

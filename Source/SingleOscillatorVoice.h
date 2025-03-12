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
   double mVol{ .25 };
   double mPulseWidth{ .5 };
   Oscillator::SyncMode mSyncMode{ Oscillator::SyncMode::None };
   double mSyncFreq{ 200 };
   double mSyncRatio{ 1 };
   double mMult{ 1 };
   OscillatorType mOscType{ OscillatorType::kOsc_Square };
   double mDetune{ 0 };
   double mShuffle{ 0 };
   double mPhaseOffset{ 0 };
   int mUnison{ 1 };
   double mUnisonWidth{ 0 };
   double mSoften{ 0 };

   double mFilterCutoffMax{ SINGLEOSCILLATOR_NO_CUTOFF };
   double mFilterCutoffMin{ 10 };
   double mFilterQ{ sqrt(2) / 2 };
   ::ADSR mFilterAdsr{ 1, 0, 1, 1000 };

   double mVelToVolume{ 1.0 };
   double mVelToEnvelope{ 0 };

   bool mLiteCPUMode{ false };
};

class SingleOscillatorVoice : public IMidiVoice
{
public:
   SingleOscillatorVoice(IDrawableModule* owner = nullptr);
   ~SingleOscillatorVoice();

   // IMidiVoice
   void Start(double time, double target) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out, int oversampling) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;

   static double GetADSRScale(double velocity, double velToEnvelope);

   static const int kMaxUnison = 8;

private:
   void DoParameterUpdate(int samplesIn,
                          double& pitch,
                          double& freq,
                          double& vol,
                          double& syncPhaseInc);

   struct OscData
   {
      double mPhase{ 0 };
      double mSyncPhase{ 0 };
      Oscillator mOsc{ kOsc_Square };
      double mDetuneFactor{ 0 };
      double mCurrentPhaseInc{ 0 };
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

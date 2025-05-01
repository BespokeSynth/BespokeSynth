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
//  KarplusStrongVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#pragma once

#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"
#include "RollingBuffer.h"
#include "Ramp.h"

class IDrawableModule;
class KarplusStrong;

enum KarplusStrongSourceType
{
   kSourceTypeSin,
   kSourceTypeNoise,
   kSourceTypeMix,
   kSourceTypeSaw,
   kSourceTypeInput,
   kSourceTypeInputNoEnvelope
};

class KarplusStrongVoiceParams : public IVoiceParams
{
public:
   KarplusStrongVoiceParams()
   {
   }
   float mFilter{ 1 };
   float mFeedback{ .98 };
   KarplusStrongSourceType mSourceType{ KarplusStrongSourceType::kSourceTypeMix };
   bool mInvert{ false };
   float mExciterFreq{ 100 };
   float mExciterAttack{ 1 };
   float mExciterDecay{ 3 };
   float mExcitation{ 0 };
   float mPitchTone{ 0 };
   float mVelToVolume{ 1.0 };
   float mVelToEnvelope{ .5 };
   bool mLiteCPUMode{ false };
};

class KarplusStrongVoice : public IMidiVoice
{
public:
   KarplusStrongVoice(IDrawableModule* owner = nullptr);
   ~KarplusStrongVoice();

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out, int oversampling) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;

private:
   void DoParameterUpdate(int samplesIn,
                          int oversampling,
                          float& pitch,
                          float& freq,
                          float& filterRate,
                          float& filterLerp,
                          float& oscPhaseInc);

   float mOscPhase{ 0 };
   EnvOscillator mOsc{ OscillatorType::kOsc_Sin };
   ::ADSR mEnv;
   KarplusStrongVoiceParams* mVoiceParams{ nullptr };
   RollingBuffer mBuffer;
   float mFilteredSample{ 0 };
   Ramp mMuteRamp;
   float mLastBufferSample{ 0 };
   bool mActive{ false };
   IDrawableModule* mOwner{ nullptr };
   KarplusStrong* mKarplusStrongModule{ nullptr };
};

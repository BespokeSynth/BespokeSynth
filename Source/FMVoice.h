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
//  FMVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#pragma once

#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class FMVoiceParams : public IVoiceParams
{
public:
   ::ADSR mOscADSRParams;
   ::ADSR mModIdxADSRParams;
   ::ADSR mHarmRatioADSRParams;
   ::ADSR mModIdxADSRParams2;
   ::ADSR mHarmRatioADSRParams2;
   float mModIdx{ 0 };
   float mHarmRatio{ 1 };
   float mModIdx2{ 0 };
   float mHarmRatio2{ 1 };
   float mVol{ 1 };
   float mPhaseOffset0{ 0 };
   float mPhaseOffset1{ 0 };
   float mPhaseOffset2{ 0 };
};

class FMVoice : public IMidiVoice
{
public:
   FMVoice(IDrawableModule* owner = nullptr);
   ~FMVoice();

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out, int oversampling) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;

private:
   float mOscPhase{ 0 };
   EnvOscillator mOsc{ kOsc_Sin };
   float mHarmPhase{ 0 };
   EnvOscillator mHarm{ kOsc_Sin };
   ::ADSR mModIdx;
   float mHarmPhase2{ 0 };
   EnvOscillator mHarm2{ kOsc_Sin };
   ::ADSR mModIdx2;
   FMVoiceParams* mVoiceParams{ nullptr };
   IDrawableModule* mOwner;
};

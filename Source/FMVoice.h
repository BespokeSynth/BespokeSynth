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

#ifndef __modularSynth__FMVoice__
#define __modularSynth__FMVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
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
   float mModIdx;
   float mHarmRatio;
   float mModIdx2;
   float mHarmRatio2;
   float mVol;
   float mPhaseOffset0;
   float mPhaseOffset1;
   float mPhaseOffset2;
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
   float mOscPhase;
   EnvOscillator mOsc;
   float mHarmPhase;
   EnvOscillator mHarm;
   ::ADSR mModIdx;
   float mHarmPhase2;
   EnvOscillator mHarm2;
   ::ADSR mModIdx2;
   FMVoiceParams* mVoiceParams;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__FMVoice__) */

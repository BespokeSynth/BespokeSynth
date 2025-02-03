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
//  SampleVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#pragma once

#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class SampleVoiceParams : public IVoiceParams
{
public:
   ::ADSR mAdsr{ 1, 0, 1, 10 };
   float mVol{ 1 };
   Sample* mSample{ nullptr };
   float mSamplePitch{ 48 };
   int mStartSample{ 0 };
   int mStopSample{ -1 };
   int mSustainLoopStart{ -1 };
   int mSustainLoopEnd{ -1 };
};

class SampleVoice : public IMidiVoice
{
public:
   SampleVoice(IDrawableModule* owner = nullptr);
   ~SampleVoice();

   float GetSamplePosition() const { return mPos; }

   // IMidiVoice
   void Start(double time, float amount) override;
   void Stop(double time) override;
   void ClearVoice() override;
   bool Process(double time, ChannelBuffer* out, int oversampling) override;
   void SetVoiceParams(IVoiceParams* params) override;
   bool IsDone(double time) override;

private:
   ::ADSR mAdsr;
   SampleVoiceParams* mVoiceParams{};
   float mPos{ 0 };
   IDrawableModule* mOwner{ nullptr };
};

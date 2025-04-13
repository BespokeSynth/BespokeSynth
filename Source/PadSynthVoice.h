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
//  PadSynthVoice.h
//  modularSynth
//
//  Created by Andrius Merkys on 4/8/25.
//
//

#pragma once

#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"
#include "RollingBuffer.h"

class FFT;
class IDrawableModule;

class PadSynthVoiceParams : public IVoiceParams
{
public:
   PadSynthVoiceParams()
   {
   }
   ::ADSR mAdsr{ 200, 0, 1, 400 };

   float mFilter{ 1 };
   float mFeedback{ .98 };
   float mPitchTone{ 0 };
   float mVelToVolume{ 1.0 };
   float mVelToEnvelope{ .5 };
   bool mLiteCPUMode{ true };
   float mBandwidth{ 40.0 };
   int mHarmonics{ 64 };
   float mBandwidthScale{ 1.0 };
   float mDetune{ 0.0 };
   float mChannelOffset{ 0.5 };
};

class PadSynthVoice : public IMidiVoice
{
public:
   PadSynthVoice(IDrawableModule* owner = nullptr);
   ~PadSynthVoice();

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
                          float& freq);

   ::ADSR mAdsr;
   PadSynthVoiceParams* mVoiceParams{ nullptr };
   IDrawableModule* mOwner{ nullptr };
   ::FFT* mFFT;

   int mUndersample{ 1 };
   int mSample{ 0 };
};

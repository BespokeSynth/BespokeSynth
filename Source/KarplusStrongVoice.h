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

#ifndef __modularSynth__KarplusStrongVoice__
#define __modularSynth__KarplusStrongVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
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
   : mFilter(1)
   , mFeedback(.98f)
   , mSourceType(kSourceTypeMix)
   , mInvert(false)
   , mExciterFreq(100)
   , mExciterAttack(1)
   , mExciterDecay(3)
   , mExcitation(0)
   , mPitchTone(0)
   , mVelToVolume(.5f)
   , mVelToEnvelope(.5f)
   , mLiteCPUMode(false)
   {}
   float mFilter;
   float mFeedback;
   KarplusStrongSourceType mSourceType;
   bool mInvert;
   float mExciterFreq;
   float mExciterAttack;
   float mExciterDecay;
   float mExcitation;
   float mPitchTone;
   float mVelToVolume;
   float mVelToEnvelope;
   bool mLiteCPUMode;
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
   
   float mOscPhase;
   EnvOscillator mOsc;
   ::ADSR mEnv;
   KarplusStrongVoiceParams* mVoiceParams;
   RollingBuffer mBuffer;
   float mFilteredSample;
   Ramp mMuteRamp;
   float mLastBufferSample;
   bool mActive;
   IDrawableModule* mOwner;
   KarplusStrong* mKarplusStrongModule;
};

#endif /* defined(__modularSynth__KarplusStrongVoice__) */

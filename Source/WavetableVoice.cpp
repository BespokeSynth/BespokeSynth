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
//  WavetableVoice.cpp
//  Bespoke
//

#include "WavetableVoice.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"
#include "ChannelBuffer.h"

WavetableVoice::WavetableVoice(IDrawableModule* owner)
: mOwner(owner)
{
}

WavetableVoice::~WavetableVoice()
{
}

bool WavetableVoice::IsDone(double time)
{
   //the voice is only finished once BOTH oscillators' independent amp envelopes have
   //finished releasing (each osc now has its own env)
   return mAdsr.IsDone(time) && mAdsrB.IsDone(time);
}

bool WavetableVoice::Process(double time, ChannelBuffer* out, int oversampling)
{
   PROFILER(WavetableVoice);

   if (IsDone(time))
      return false;

   bool mono = (out->NumActiveChannels() == 1);

   float pitch;
   float freq;
   float vol;
   float syncPhaseInc;

   if (mVoiceParams->mLiteCPUMode)
      DoParameterUpdate(0, pitch, freq, vol, syncPhaseInc);

   WavetableFrameSet* tableA = mVoiceParams->mTableA.get();
   WavetableFrameSet* tableB = mVoiceParams->mTableB.get();
   bool useA = mVoiceParams->mUseOscA && tableA != nullptr;
   bool useB = mVoiceParams->mUseOscB && tableB != nullptr;

   //per-voice spread: nudge this voice's scan position and filter cutoff by its own stable random
   //offset, scaled by mVoiceSpread. With several voices held, they drift apart for a richer stack.
   const float spread = mVoiceParams->mVoiceSpread;
   const float posA = ofClamp(mVoiceParams->mPositionA + mVoiceRandPos * spread * 0.5f, 0.0f, 1.0f);
   const float posB = ofClamp(mVoiceParams->mPositionB + mVoiceRandPos * spread * 0.5f, 0.0f, 1.0f);
   const float cutMult = 1.0f + mVoiceRandCut * spread * 0.5f;

   for (int pos = 0; pos < out->BufferSize(); ++pos)
   {
      if (!mVoiceParams->mLiteCPUMode)
         DoParameterUpdate(pos, pitch, freq, vol, syncPhaseInc);

      float adsrValA = mAdsr.Value(time);
      float adsrValB = mAdsrB.Value(time);

      //advance oscillator A's unison phases first, so we know if voice 0 wrapped
      //this sample (used as the hard-sync trigger for oscillator B)
      bool voice0Wrapped = false;
      for (int u = 0; u < mVoiceParams->mUnison && u < kMaxUnison; ++u)
      {
         mOscData[u].mPhase += mOscData[u].mCurrentPhaseInc;
         if (std::isinf(mOscData[u].mPhase))
         {
            ofLog() << "Infinite phase. phaseInc:" + ofToString(mOscData[u].mCurrentPhaseInc) + " detune:" + ofToString(mVoiceParams->mDetune) + " freq:" + ofToString(freq) + " pitch:" + ofToString(pitch) + " getpitch:" + ofToString(GetPitch(pos));
            mOscData[u].mPhase = 0;
            mOscData[u].mCurrentPhaseInc = 0;
         }
         else
         {
            while (mOscData[u].mPhase > FTWO_PI * 2)
            {
               mOscData[u].mPhase -= FTWO_PI * 2;
               mOscData[u].mSyncPhase = 0;
               if (u == 0)
                  voice0Wrapped = true;
            }
         }
         mOscData[u].mSyncPhase += syncPhaseInc;
         if (std::isinf(mOscData[u].mSyncPhase))
         {
            mOscData[u].mSyncPhase = 0;
            syncPhaseInc = 0;
         }
      }

      //oscillator B: now a full unison voice too, mirroring A. all of its voices
      //can hard-sync to A's cycle (voice 0's wrap), and voice 0's raw sample is
      //used below as the representative modulator signal for cross-modulating A
      float bSample = 0;
      if (useB)
      {
         for (int u = 0; u < mVoiceParams->mUnisonB && u < kMaxUnison; ++u)
         {
            if (mVoiceParams->mSyncB && voice0Wrapped)
            {
               mOscDataB[u].mPhase = 0;
            }
            else
            {
               mOscDataB[u].mPhase += mOscDataB[u].mCurrentPhaseInc;
               if (std::isinf(mOscDataB[u].mPhase))
                  mOscDataB[u].mPhase = 0;
            }
            while (mOscDataB[u].mPhase > FTWO_PI)
               mOscDataB[u].mPhase -= FTWO_PI;
            while (mOscDataB[u].mPhase < 0)
               mOscDataB[u].mPhase += FTWO_PI;
         }

         bSample = WavetableTables::ReadWarped(tableB, posB, mOscDataB[0].mPhase, mVoiceParams->mWarpType, mVoiceParams->mWarpAmount);
      }

      //osc A and osc B are summed on SEPARATE signal paths now, so each can be run through
      //its own amp envelope and its own filter before being mixed together at the end
      float summedLeftA = 0;
      float summedRightA = 0;
      float summedLeftB = 0;
      float summedRightB = 0;
      for (int u = 0; useA && u < mVoiceParams->mUnison && u < kMaxUnison; ++u) //osc A can be switched off
      {
         float readPhase = (mVoiceParams->mSyncMode != Oscillator::SyncMode::None)
                           ? mOscData[u].mSyncPhase
                           : (mOscData[u].mPhase + mVoiceParams->mPhaseOffset * (1 + (float(u) / mVoiceParams->mUnison)));

         float rawA;
         if (useB && mVoiceParams->mModType == WavetableModType::FM)
         {
            //digital "FM" is, in practice, phase modulation - offset the read phase
            //directly by B's (linear) sample value
            float modulatedPhase = readPhase + bSample * mVoiceParams->mModAmount * FTWO_PI * 2.0f;
            rawA = WavetableTables::ReadWarped(tableA, posA, modulatedPhase, mVoiceParams->mWarpType, mVoiceParams->mWarpAmount);
         }
         else if (useB && mVoiceParams->mModType == WavetableModType::PD)
         {
            //phase distortion: shape B's sample nonlinearly before using it as a
            //phase offset, so it sounds distinct from the FM mode above
            float shaped = (bSample < 0 ? -1.0f : 1.0f) * bSample * bSample;
            float modulatedPhase = readPhase + shaped * mVoiceParams->mModAmount * FTWO_PI * 0.5f;
            rawA = WavetableTables::ReadWarped(tableA, posA, modulatedPhase, mVoiceParams->mWarpType, mVoiceParams->mWarpAmount);
         }
         else
         {
            rawA = WavetableTables::ReadWarped(tableA, posA, readPhase, mVoiceParams->mWarpType, mVoiceParams->mWarpAmount);
            if (useB && mVoiceParams->mModType == WavetableModType::AM)
               rawA *= ofLerp(1.0f, (bSample * 0.5f + 0.5f) * 2.0f, mVoiceParams->mModAmount);
            else if (useB && mVoiceParams->mModType == WavetableModType::RM)
               rawA = ofLerp(rawA, rawA * bSample, mVoiceParams->mModAmount);
         }

         float sample = rawA * adsrValA * vol;

         //center voice stays loudest, edge voices roll off a touch (supersaw shaping)
         sample *= 1.0f - fabsf(mOscData[u].mDetuneFactor) * 0.3f;

         if (mono)
         {
            summedLeftA += sample;
         }
         else
         {
            //pan each unison voice by its position in the even fan, scaled by unison width
            float pan = GetPan() + mOscData[u].mDetuneFactor * mVoiceParams->mUnisonWidth;
            summedLeftA += sample * GetLeftPanGain(pan);
            summedRightA += sample * GetRightPanGain(pan);
         }
      }

      //oscillator B is also heard directly (its own full unison spread), scaled
      //by its own volume - same structure as A's summing loop just above
      if (useB && mVoiceParams->mVolB > 0)
      {
         float volB = mVoiceParams->mVolB * .4f / mVoiceParams->mUnisonB;
         for (int u = 0; u < mVoiceParams->mUnisonB && u < kMaxUnison; ++u)
         {
            float rawB = WavetableTables::ReadWarped(tableB, posB, mOscDataB[u].mPhase, mVoiceParams->mWarpType, mVoiceParams->mWarpAmount);
            float sample = rawB * adsrValB * volB;

            //center voice loudest, edges rolled off (same supersaw shaping as A)
            sample *= 1.0f - fabsf(mOscDataB[u].mDetuneFactor) * 0.3f;

            if (mono)
            {
               summedLeftB += sample;
            }
            else
            {
               float pan = GetPan() + mOscDataB[u].mDetuneFactor * mVoiceParams->mUnisonWidthB;
               summedLeftB += sample * GetLeftPanGain(pan);
               summedRightB += sample * GetRightPanGain(pan);
            }
         }
      }

      //osc A's own filter (driven by its own filter envelope), applied only to A's signal
      if (mUseFilter)
      {
         float f = ofLerp(mVoiceParams->mFilterCutoffMin, mVoiceParams->mFilterCutoffMax, mFilterAdsr.Value(time)) * (1 - GetModWheel(pos) * .9f);
         f = ofClamp(f * cutMult, 10.0f, WAVETABLE_NO_CUTOFF); //per-voice cutoff spread
         float q = mVoiceParams->mFilterQ;
         if (f != mFilterLeft.mF || q != mFilterLeft.mQ)
            mFilterLeft.SetFilterParams(f, q);
         summedLeftA = mFilterLeft.Filter(summedLeftA);
         if (!mono)
         {
            mFilterRight.CopyCoeffFrom(mFilterLeft);
            summedRightA = mFilterRight.Filter(summedRightA);
         }
      }

      //osc B's own filter (driven by B's own filter envelope), applied only to B's signal
      if (mUseFilterB)
      {
         float f = ofLerp(mVoiceParams->mFilterCutoffMinB, mVoiceParams->mFilterCutoffMaxB, mFilterAdsrB.Value(time)) * (1 - GetModWheel(pos) * .9f);
         f = ofClamp(f * cutMult, 10.0f, WAVETABLE_NO_CUTOFF); //per-voice cutoff spread
         float q = mVoiceParams->mFilterQB;
         if (f != mFilterLeftB.mF || q != mFilterLeftB.mQ)
            mFilterLeftB.SetFilterParams(f, q);
         summedLeftB = mFilterLeftB.Filter(summedLeftB);
         if (!mono)
         {
            mFilterRightB.CopyCoeffFrom(mFilterLeftB);
            summedRightB = mFilterRightB.Filter(summedRightB);
         }
      }

      {
         if (mono)
         {
            out->GetChannel(0)[pos] += summedLeftA + summedLeftB;
         }
         else
         {
            out->GetChannel(0)[pos] += summedLeftA + summedLeftB;
            out->GetChannel(1)[pos] += summedRightA + summedRightB;
         }
      }
      time += gInvSampleRateMs;
   }

   return true;
}

void WavetableVoice::DoParameterUpdate(int samplesIn,
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

   //osc A unison: spread the voices EVENLY across [-1,+1] (Serum/Vital "supersaw" fan) instead of
   //the old lopsided 1/0/random pattern, with a small stable per-voice randomisation so it isn't
   //perfectly sterile. The spread factor is stored back into mDetuneFactor so the summing loop
   //uses the same value for detune, level shaping, and stereo panning.
   for (int u = 0; u < mVoiceParams->mUnison && u < kMaxUnison; ++u)
   {
      float spread = (mVoiceParams->mUnison <= 1) ? 0.0f : ((float)u / (mVoiceParams->mUnison - 1) * 2.0f - 1.0f);
      spread = ofClamp(spread + mOscData[u].mRandomOffset * 0.15f, -1.0f, 1.0f);
      mOscData[u].mDetuneFactor = spread;
      float detune = exp2(mVoiceParams->mDetune * spread * (1 - GetPressure(samplesIn)));
      mOscData[u].mCurrentPhaseInc = GetPhaseInc(freq * detune);
   }

   //osc B unison: same even fan, using B's own unison count and detune amount
   for (int u = 0; u < mVoiceParams->mUnisonB && u < kMaxUnison; ++u)
   {
      float spreadB = (mVoiceParams->mUnisonB <= 1) ? 0.0f : ((float)u / (mVoiceParams->mUnisonB - 1) * 2.0f - 1.0f);
      spreadB = ofClamp(spreadB + mOscDataB[u].mRandomOffset * 0.15f, -1.0f, 1.0f);
      mOscDataB[u].mDetuneFactor = spreadB;
      float detuneB = exp2(mVoiceParams->mDetuneB * spreadB * (1 - GetPressure(samplesIn)));
      mOscDataB[u].mCurrentPhaseInc = GetPhaseInc(freq * detuneB);
   }
}

//static
float WavetableVoice::GetADSRScale(float velocity, float velToEnvelope)
{
   if (velToEnvelope >= 0)
      return ofLerp(ofClamp(1 - velToEnvelope, 0, 1), 1, velocity);
   return ofClamp(ofLerp(1, 1 + velToEnvelope, velocity), 0.001f, 1);
}

//static
float WavetableVoice::GetADSRCurve(float velocity, float velToEnvelope)
{
   if (velToEnvelope < -1)
      return -(velToEnvelope + 1) * 0.25f;
   if (velToEnvelope > 1)
      return -(velToEnvelope - 1) * 0.25f;
   return 0;
}

void WavetableVoice::Start(double time, float target)
{
   if (mVoiceParams->mVelToVolume > 1)
      target = pow(target, mVoiceParams->mVelToVolume);
   float volume = ofLerp(MAX(0, 1 - mVoiceParams->mVelToVolume), MAX(1, mVoiceParams->mVelToVolume), target);
   float cutoffScale = 1 + MAX(0, mVoiceParams->mVelToEnvelope - 1);
   float adsrTimeScale = GetADSRScale(target, mVoiceParams->mVelToEnvelope);
   float adsrCurve = GetADSRCurve(target, mVoiceParams->mVelToEnvelope);
   mAdsr.Start(time, volume, mVoiceParams->mAdsr, 1, adsrCurve);

   if (mVoiceParams->mFilterCutoffMax != WAVETABLE_NO_CUTOFF)
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

   //osc B's independent amp env + filter env start with the same velocity response as A's,
   //but read B's own ADSR/filter parameters so B is shaped and filtered on its own
   mAdsrB.Start(time, volume, mVoiceParams->mAdsrB, 1, adsrCurve);

   if (mVoiceParams->mFilterCutoffMaxB != WAVETABLE_NO_CUTOFF)
   {
      mUseFilterB = true;
      mFilterLeftB.SetFilterType(kFilterType_Lowpass);
      mFilterRightB.SetFilterType(kFilterType_Lowpass);
      mFilterAdsrB.Start(time, cutoffScale, mVoiceParams->mFilterAdsrB, adsrTimeScale, adsrCurve);
   }
   else
   {
      mUseFilterB = false;
   }
}

void WavetableVoice::Stop(double time)
{
   mAdsr.Stop(time);
   mFilterAdsr.Stop(time);
   mAdsrB.Stop(time);
   mFilterAdsrB.Stop(time);
}

void WavetableVoice::ClearVoice()
{
   mAdsr.Clear();
   mFilterAdsr.Clear();
   mAdsrB.Clear();
   mFilterAdsrB.Clear();

   //fresh per-voice random offsets for the voice-spread feature (each note gets its own drift)
   mVoiceRandCut = ofRandom(-1, 1);
   mVoiceRandPos = ofRandom(-1, 1);

   for (int u = 0; u < kMaxUnison; ++u)
   {
      //decorrelate each unison voice's starting phase. This is what makes unison audibly "fatten"
      //even at low/zero detune (the stacked copies no longer sum to a single phase-locked tone),
      //and it makes the stereo spread meaningful. mDetuneFactor itself is (re)computed every param
      //update in DoParameterUpdate as an even fan; the per-voice random offset kept here just adds
      //a stable, subtle imperfection so the fan isn't perfectly sterile.
      mOscData[u].mPhase = ofRandom(0, FTWO_PI);
      mOscData[u].mSyncPhase = 0;
      mOscDataB[u].mPhase = ofRandom(0, FTWO_PI);

      mOscData[u].mRandomOffset = ofRandom(-1, 1);
      mOscDataB[u].mRandomOffset = ofRandom(-1, 1);
   }
}

void WavetableVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<WavetableVoiceParams*>(params);
}

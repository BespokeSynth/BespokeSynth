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
//  WavetableVoice.h
//  Bespoke
//
//  Voice/param plumbing for the Wavetable module (rebuilt to be closer to how
//  Ableton's Wavetable device and Serum/Vital-style synths actually work):
//   - Oscillator A and Oscillator B are now full, symmetric voices - each has
//     its own table/position/volume/detune/unison/unison-width, mirroring the
//     existing (non-wavetable) Oscillator module's controls one for one.
//   - B can hard-sync to A and cross-modulate it (FM/PD/AM/RM), and is also
//     heard directly via its own volume level.
//   - The filter (with its own envelope) and the amp envelope are shared
//     between A and B (applied to the combined signal), rather than
//     duplicated per-oscillator, to keep this reasonably simple.
//   - A shared Serum-style Warp stage (Bend/Asym/Flip/Mirror/Quantize/Odd/
//     Even) applies to whichever table is currently sounding.
//

#pragma once

#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "LFO.h"
#include "BiquadFilter.h"
#include "WavetableTables.h"
#include <memory>

#define WAVETABLE_NO_CUTOFF 10000

enum class WavetableModType
{
   None,
   FM,
   PD,
   AM,
   RM
};

class WavetableVoiceParams : public IVoiceParams
{
public:
   ::ADSR mAdsr{ 10, 0, 1, 10 };
   float mVol{ .25 };

   Oscillator::SyncMode mSyncMode{ Oscillator::SyncMode::None };
   float mSyncFreq{ 200 };
   float mSyncRatio{ 1 };
   float mMult{ 1 };
   float mDetune{ 0 };
   float mPhaseOffset{ 0 };
   int mUnison{ 1 };
   float mUnisonWidth{ 0 };

   float mFilterCutoffMax{ WAVETABLE_NO_CUTOFF };
   float mFilterCutoffMin{ 10 };
   float mFilterQ{ float(sqrt(2) / 2) };
   ::ADSR mFilterAdsr{ 1, 0, 1, 1000 };

   float mVelToVolume{ 1.0 };
   float mVelToEnvelope{ 0 };

   bool mLiteCPUMode{ false };

   //oscillator A - the main voice, always on
   std::shared_ptr<WavetableFrameSet> mTableA;
   float mPositionA{ 0 };

   //oscillator B - a second full oscillator, mirroring A's own controls one for
   //one (volume/detune/unison/width all mean the same thing as A's), off by
   //default. can hard-sync to A and cross-modulate it, and is also heard
   //directly via its own volume level.
   bool mUseOscB{ false };
   std::shared_ptr<WavetableFrameSet> mTableB;
   float mPositionB{ 0 };
   float mVolB{ .25f };
   float mDetuneB{ 0 }; //unison spread amount - same semantics/range as mDetune
   int mUnisonB{ 1 };
   float mUnisonWidthB{ 0 };
   bool mSyncB{ false }; //hard-sync B's phase to A's cycle

   //osc B's own amp envelope + filter + filter-envelope, fully independent from A's.
   //Each oscillator is now shaped and filtered on its own signal path before the two are
   //summed, matching Serum/Vital/Ableton-style per-oscillator routing.
   ::ADSR mAdsrB{ 10, 0, 1, 10 };
   float mFilterCutoffMaxB{ WAVETABLE_NO_CUTOFF };
   float mFilterCutoffMinB{ 10 };
   float mFilterQB{ float(sqrt(2) / 2) };
   ::ADSR mFilterAdsrB{ 1, 0, 1, 1000 };

   WavetableModType mModType{ WavetableModType::None };
   float mModAmount{ 0 };

   WavetableWarpType mWarpType{ WavetableWarpType::None };
   float mWarpAmount{ 0.5f };

   //osc A can be switched on/off too (both oscillators now have an enable), default on
   bool mUseOscA{ true };

   //synth-wide voice richness: per-voice random variation of filter cutoff + wavetable scan
   //position (0 = all voices identical). With several voices this makes them drift apart for a
   //fatter, more analog / Pigments-style stack.
   float mVoiceSpread{ 0 };
};

class WavetableVoice : public IMidiVoice
{
public:
   WavetableVoice(IDrawableModule* owner = nullptr);
   ~WavetableVoice();

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
      float mDetuneFactor{ 0 }; //even symmetric unison spread position in [-1,1], recomputed per param update
      float mRandomOffset{ 0 }; //small stable per-voice randomisation so the fan isn't perfectly sterile
      float mCurrentPhaseInc{ 0 };
   };
   OscData mOscData[kMaxUnison]; //oscillator A's unison voices
   OscData mOscDataB[kMaxUnison]; //oscillator B's unison voices (mSyncPhase unused - B doesn't have its own sync-mode selector, only the hard-sync-to-A checkbox)
   ::ADSR mAdsr;
   WavetableVoiceParams* mVoiceParams{ nullptr };

   ::ADSR mFilterAdsr;
   BiquadFilter mFilterLeft;
   BiquadFilter mFilterRight;
   bool mUseFilter{ false };

   //osc B's independent amp env, filter env, and stereo filter pair
   ::ADSR mAdsrB;
   ::ADSR mFilterAdsrB;
   BiquadFilter mFilterLeftB;
   BiquadFilter mFilterRightB;
   bool mUseFilterB{ false };

   //stable per-voice random offsets (rolled fresh each note) scaled by mVoiceSpread
   float mVoiceRandCut{ 0 };
   float mVoiceRandPos{ 0 };

   IDrawableModule* mOwner;
};

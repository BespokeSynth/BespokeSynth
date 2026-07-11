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
//  WavetableTables.h
//  Bespoke
//
//  Multi-frame wavetable data (modeled on how Ableton's Wavetable device
//  works: a table is just a stack of same-length single-cycle frames, and a
//  "position" scans/morphs across them). Two sources of tables:
//   - built-in tables, generated procedurally at startup (no external assets
//     to ship): "basic" (sine/triangle/saw/square morph), "harmonics"
//     (sine -> buzzy saw-ish sweep by adding harmonics), "pwm" (narrow to
//     wide pulse sweep), "formant" (vowel-ish sweep, reusing the same formant
//     band idea as FormantFilterEffect).
//   - imported tables, built from a dropped-in audio sample the same way
//     Ableton does it: no pitch detection, just slice the raw audio into up
//     to kMaxImportedFrames equal consecutive windows and resample each
//     window to kTableSize samples.
//
//  Each frame also gets a lazily-computed odd/even-harmonics-only version
//  (via a small one-shot DFT/IDFT, since kTableSize is tiny) so the "warp"
//  stage in WavetableVoice can offer Odd/Even like Serum's wavetable warp
//  modes do.
//

#pragma once

#include <vector>
#include <string>
#include <memory>

class WavetableFrameSet
{
public:
   static const int kTableSize = 256;
   static const int kMaxImportedFrames = 64;

   explicit WavetableFrameSet(std::string name)
   : mName(std::move(name))
   {
   }

   const std::string& GetName() const { return mName; }
   int GetFrameCount() const { return (int)mFrames.size(); }

   //adds one frame (must be exactly kTableSize samples); ownership stays here
   void AddFrame(const std::vector<float>& frame);

   //call once after all frames are added - computes the odd/even harmonic-only
   //caches used by the Odd/Even warp modes. Cheap enough to do once per table
   //(a handful of frames at most, kTableSize=256), never done per-audio-sample.
   void FinalizeHarmonicCaches();

   //returns a pointer to kTableSize floats for the given frame index (clamped),
   //choosing the odd-only/even-only cache if requested
   const float* GetFrameData(int frameIndex, bool oddOnly, bool evenOnly) const;

private:
   std::string mName;
   std::vector<std::vector<float>> mFrames; //[frame][sample], each inner vector is kTableSize long
   std::vector<std::vector<float>> mFramesOddOnly;
   std::vector<std::vector<float>> mFramesEvenOnly;
};

enum class WavetableWarpType
{
   None,
   BendPlus,
   BendMinus,
   BendBoth,
   AsymPlus,
   AsymMinus,
   AsymBoth,
   Flip,
   Mirror,
   Quantize,
   OddOnly,
   EvenOnly,
   Count
};

class WavetableTables
{
public:
   //returns the shared, lazily-built set of built-in tables (indices are stable
   //and match the order the Wavetable module's dropdowns list them in)
   static const std::vector<std::shared_ptr<WavetableFrameSet>>& GetBuiltInTables();

   //Ableton-style import: no pitch detection, just slice consecutive equal
   //windows out of the raw mono audio and resample each to kTableSize samples
   static std::shared_ptr<WavetableFrameSet> BuildFromSample(const float* data, int numSamples, const std::string& name);

   //reads one interpolated sample out of a table at a given scan position
   //(0-1, blended across frames) and phase (0-FTWO_PI), applying the given
   //warp. Shared by both oscillator A and oscillator B in WavetableVoice.
   static float ReadWarped(const WavetableFrameSet* table, float position, float phase, WavetableWarpType warp, float warpAmount);

private:
   static std::vector<float> ResampleWindow(const float* data, int start, int length, int targetLength);
};

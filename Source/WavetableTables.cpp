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
//  WavetableTables.cpp
//  Bespoke
//

#include "WavetableTables.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include <cmath>
#include <algorithm>

namespace
{
   const int N = WavetableFrameSet::kTableSize;

   float NaiveSine(float phase01) { return sinf(phase01 * FTWO_PI); }
   float NaiveTriangle(float phase01)
   {
      float t = phase01 - floorf(phase01);
      return 1.0f - 4.0f * fabsf(t - 0.5f);
   }
   float NaiveSaw(float phase01)
   {
      float t = phase01 - floorf(phase01);
      return 2.0f * t - 1.0f;
   }
   float NaiveSquare(float phase01, float width = 0.5f)
   {
      float t = phase01 - floorf(phase01);
      return (t < width) ? 1.0f : -1.0f;
   }
}

void WavetableFrameSet::AddFrame(const std::vector<float>& frame)
{
   std::vector<float> copy = frame;
   copy.resize(kTableSize, 0.0f);
   mFrames.push_back(std::move(copy));
}

void WavetableFrameSet::FinalizeHarmonicCaches()
{
   mFramesOddOnly.clear();
   mFramesEvenOnly.clear();
   mFramesOddOnly.reserve(mFrames.size());
   mFramesEvenOnly.reserve(mFrames.size());

   for (const auto& frame : mFrames)
   {
      //one-shot naive DFT/IDFT - only ever run at table-build time (a handful of
      //frames, kTableSize samples each), never on the audio thread
      std::vector<float> a(N / 2, 0.0f);
      std::vector<float> b(N / 2, 0.0f);
      for (int k = 1; k < N / 2; ++k)
      {
         float sumCos = 0;
         float sumSin = 0;
         for (int n = 0; n < N; ++n)
         {
            float angle = FTWO_PI * k * n / N;
            sumCos += frame[n] * cosf(angle);
            sumSin += frame[n] * sinf(angle);
         }
         a[k] = sumCos * 2.0f / N;
         b[k] = sumSin * 2.0f / N;
      }

      std::vector<float> oddOnly(N, 0.0f);
      std::vector<float> evenOnly(N, 0.0f);
      for (int n = 0; n < N; ++n)
      {
         float oddSum = 0;
         float evenSum = 0;
         for (int k = 1; k < N / 2; ++k)
         {
            float angle = FTWO_PI * k * n / N;
            float contribution = a[k] * cosf(angle) + b[k] * sinf(angle);
            if (k % 2 == 1)
               oddSum += contribution;
            else
               evenSum += contribution;
         }
         oddOnly[n] = oddSum;
         evenOnly[n] = evenSum;
      }
      mFramesOddOnly.push_back(std::move(oddOnly));
      mFramesEvenOnly.push_back(std::move(evenOnly));
   }
}

const float* WavetableFrameSet::GetFrameData(int frameIndex, bool oddOnly, bool evenOnly) const
{
   if (mFrames.empty())
      return nullptr;
   frameIndex = ofClamp(frameIndex, 0, (int)mFrames.size() - 1);
   if (oddOnly && frameIndex < (int)mFramesOddOnly.size())
      return mFramesOddOnly[frameIndex].data();
   if (evenOnly && frameIndex < (int)mFramesEvenOnly.size())
      return mFramesEvenOnly[frameIndex].data();
   return mFrames[frameIndex].data();
}

std::vector<float> WavetableTables::ResampleWindow(const float* data, int start, int length, int targetLength)
{
   std::vector<float> out(targetLength, 0.0f);
   if (length <= 0)
      return out;
   for (int j = 0; j < targetLength; ++j)
   {
      float srcPos = (float)j / targetLength * length;
      int i0 = (int)srcPos;
      int i1 = std::min(i0 + 1, length - 1);
      float frac = srcPos - i0;
      float s0 = data[start + i0];
      float s1 = data[start + i1];
      out[j] = s0 + (s1 - s0) * frac;
   }
   return out;
}

std::shared_ptr<WavetableFrameSet> WavetableTables::BuildFromSample(const float* data, int numSamples, const std::string& name)
{
   auto table = std::make_shared<WavetableFrameSet>(name);

   if (data == nullptr || numSamples <= 0)
   {
      //fall back to a flat silent single frame rather than crashing on a bad/empty sample
      std::vector<float> silence(WavetableFrameSet::kTableSize, 0.0f);
      table->AddFrame(silence);
      table->FinalizeHarmonicCaches();
      return table;
   }

   //Ableton-style: no pitch detection, just slice the raw audio into consecutive
   //equal windows and resample each one to kTableSize samples. Aim for roughly
   //one frame per kTableSize samples of source content, always at least 2 frames
   //so the position knob always has something to scan across.
   int frameCount = numSamples / WavetableFrameSet::kTableSize;
   frameCount = ofClamp(frameCount, 2, WavetableFrameSet::kMaxImportedFrames);
   int windowLen = numSamples / frameCount;

   for (int i = 0; i < frameCount; ++i)
   {
      int start = i * windowLen;
      int length = (i == frameCount - 1) ? (numSamples - start) : windowLen;
      std::vector<float> frame = ResampleWindow(data, start, length, WavetableFrameSet::kTableSize);

      //remove DC offset and normalize peak so frames don't wildly differ in
      //loudness as you scan the position knob
      float mean = 0;
      for (float s : frame)
         mean += s;
      mean /= (float)frame.size();
      float peak = 0.0001f;
      for (float& s : frame)
      {
         s -= mean;
         peak = std::max(peak, fabsf(s));
      }
      float normalizeScale = 0.9f / peak;
      for (float& s : frame)
         s *= normalizeScale;

      table->AddFrame(frame);
   }

   table->FinalizeHarmonicCaches();
   return table;
}

//static
const std::vector<std::shared_ptr<WavetableFrameSet>>& WavetableTables::GetBuiltInTables()
{
   static std::vector<std::shared_ptr<WavetableFrameSet>> sTables;
   if (!sTables.empty())
      return sTables;

   {
      //"basic": classic sine -> triangle -> saw -> square morph
      auto table = std::make_shared<WavetableFrameSet>("basic");
      const int kFrames = 8;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1); //0..1 across the whole morph
         std::vector<float> frame(N);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value;
            if (t < 1.0f / 3)
               value = ofLerp(NaiveSine(phase01), NaiveTriangle(phase01), t * 3);
            else if (t < 2.0f / 3)
               value = ofLerp(NaiveTriangle(phase01), NaiveSaw(phase01), (t - 1.0f / 3) * 3);
            else
               value = ofLerp(NaiveSaw(phase01), NaiveSquare(phase01), (t - 2.0f / 3) * 3);
            frame[n] = value;
         }
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"harmonics": sine -> buzzy sawtooth-ish sweep by adding harmonics one at a time
      auto table = std::make_shared<WavetableFrameSet>("harmonics");
      const int kFrames = 16;
      for (int i = 0; i < kFrames; ++i)
      {
         int numHarmonics = i + 1;
         std::vector<float> frame(N, 0.0f);
         float ampSum = 0;
         for (int k = 1; k <= numHarmonics; ++k)
            ampSum += 1.0f / k;
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= numHarmonics; ++k)
               value += (1.0f / k) * sinf(phase01 * FTWO_PI * k);
            frame[n] = value / ampSum;
         }
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"pwm": narrow to wide pulse sweep
      auto table = std::make_shared<WavetableFrameSet>("pwm");
      const int kFrames = 16;
      for (int i = 0; i < kFrames; ++i)
      {
         float width = ofLerp(0.05f, 0.95f, (float)i / (kFrames - 1));
         std::vector<float> frame(N);
         for (int n = 0; n < N; ++n)
            frame[n] = NaiveSquare((float)n / N, width);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"formant": a vowel-ish sweep - approximate, not derived from real formant
      //frequency tables (those depend on absolute pitch, which a fixed single-
      //cycle table doesn't know about), just clusters of boosted harmonics that
      //drift as you scan, giving it a vocal/talking character
      auto table = std::make_shared<WavetableFrameSet>("formant");
      const int kFrames = 8;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float peak1 = ofLerp(2, 6, t); //lower formant drifts up
         float peak2 = ofLerp(14, 9, t); //upper formant drifts down - they cross paths across the sweep
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = sinf(phase01 * FTWO_PI); //fundamental for grounding
            for (int k = 2; k <= 20; ++k)
            {
               float d1 = fabsf(k - peak1);
               float d2 = fabsf(k - peak2);
               float weight = 0.9f * expf(-d1 * d1 / 3.0f) + 0.7f * expf(-d2 * d2 / 3.0f);
               value += (weight / k) * sinf(phase01 * FTWO_PI * k);
            }
            frame[n] = value;
         }
         //normalize
         float peak = 0.0001f;
         for (float s : frame)
            peak = std::max(peak, fabsf(s));
         for (float& s : frame)
            s *= 0.9f / peak;
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   //---------------------------------------------------------------------------------------------
   //Additional presets. Wherever possible these are built by ADDITIVE synthesis (summing sine
   //harmonics) rather than naive shapes, so they're band-limited and don't alias badly when played
   //high - the standard way to build clean wavetables. A shared normalize-to-0.9 helper keeps every
   //frame at a consistent level as the position knob scans across them.
   //---------------------------------------------------------------------------------------------
   auto normalizeFrame = [](std::vector<float>& frame)
   {
      float peak = 0.0001f;
      for (float s : frame)
         peak = std::max(peak, fabsf(s));
      float scale = 0.9f / peak;
      for (float& s : frame)
         s *= scale;
   };

   {
      //"tilt saw": a full sawtooth every frame, but the spectral tilt sweeps from dark (steep
      //high-harmonic rolloff) to bright (shallow rolloff) - like sweeping a filter over a saw
      auto table = std::make_shared<WavetableFrameSet>("tilt saw");
      const int kFrames = 8;
      const int kH = 48;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float tilt = ofLerp(1.7f, 0.6f, t); //higher = darker
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= kH; ++k)
               value += (1.0f / powf((float)k, tilt)) * sinf(phase01 * FTWO_PI * k);
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"organ": Hammond-style drawbars (octave + fifth partials). Morphs from a fundamental-heavy
      //sine-ish tone to a full drawbar stack.
      auto table = std::make_shared<WavetableFrameSet>("organ");
      const int kFrames = 8;
      const int parts[] = { 1, 2, 3, 4, 6, 8 };
      const int numParts = (int)(sizeof(parts) / sizeof(parts[0]));
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int j = 0; j < numParts; ++j)
            {
               float ampLow = powf(0.55f, (float)j); //fundamental-heavy at t=0
               float amp = ofLerp(ampLow, 1.0f, t); //all drawbars up at t=1
               value += amp * sinf(phase01 * FTWO_PI * parts[j]);
            }
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"fm bell": classic 2-operator FM (a carrier phase-modulated by a fixed-ratio modulator).
      //The morph raises the modulation index from clean sine to bright metallic/bell.
      auto table = std::make_shared<WavetableFrameSet>("fm bell");
      const int kFrames = 10;
      const float ratio = 3.5f; //modulator:carrier ratio - inharmonic-ish, gives a bell/metallic edge
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float index = ofLerp(0.0f, 6.0f, t);
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase = (float)n / N * FTWO_PI;
            frame[n] = sinf(phase + index * sinf(ratio * phase));
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"phase distort": Casio CZ-style phase distortion - a sine read through a warped phase ramp
      //that bunches toward one point, morphing sine -> bright resonant/saw-like.
      auto table = std::make_shared<WavetableFrameSet>("phase distort");
      const int kFrames = 8;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float d = ofLerp(0.5f, 0.08f, t); //knee position; 0.5 = pure sine
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float x = (float)n / N;
            float pw = (x < d) ? (x * 0.5f / d) : (0.5f + (x - d) * 0.5f / (1.0f - d));
            frame[n] = sinf(pw * FTWO_PI);
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"vox": five vowel formants (a, e, i, o, u) as boosted harmonic clusters, so scanning the
      //position knob sweeps through vowel timbres. Peaks are approximate harmonic positions, not
      //pitch-accurate formant frequencies (a single-cycle table has no absolute pitch).
      auto table = std::make_shared<WavetableFrameSet>("vox");
      const float vowelPeaks[5][2] = { { 5, 8 }, { 3, 11 }, { 2, 15 }, { 3, 5 }, { 2, 4 } }; //a e i o u
      for (int v = 0; v < 5; ++v)
      {
         float peak1 = vowelPeaks[v][0];
         float peak2 = vowelPeaks[v][1];
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = sinf(phase01 * FTWO_PI); //fundamental for body
            for (int k = 2; k <= 24; ++k)
            {
               float d1 = fabsf(k - peak1);
               float d2 = fabsf(k - peak2);
               float weight = 1.0f * expf(-d1 * d1 / 3.0f) + 0.8f * expf(-d2 * d2 / 4.0f);
               value += (weight / k) * sinf(phase01 * FTWO_PI * k);
            }
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"odd square": additive odd-harmonic build, morphing sine -> band-limited square/hollow tone
      auto table = std::make_shared<WavetableFrameSet>("odd square");
      const int kFrames = 12;
      for (int i = 0; i < kFrames; ++i)
      {
         int maxOdd = 1 + i * 2; //1,3,5,... highest odd harmonic included
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= maxOdd; k += 2)
               value += (1.0f / k) * sinf(phase01 * FTWO_PI * k);
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"harmonic sweep": a single resonant harmonic peak that sweeps upward across the frames (plus
      //a faint saw body), giving a vocal/whistle "wooo" movement as you scan the position knob
      auto table = std::make_shared<WavetableFrameSet>("harmonic sweep");
      const int kFrames = 16;
      const int kH = 48;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float peak = ofLerp(1.0f, 40.0f, t);
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= kH; ++k)
            {
               float d = fabsf(k - peak);
               float amp = expf(-d * d / 2.0f) + 0.08f / k; //sharp moving peak + faint body
               value += amp * sinf(phase01 * FTWO_PI * k);
            }
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"exp decay": harmonics with an exponential amplitude decay, morphing dark -> bright. Very
      //smooth/clean (like a resonance-free filtered saw); a great neutral workhorse table.
      auto table = std::make_shared<WavetableFrameSet>("exp decay");
      const int kFrames = 8;
      const int kH = 64;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float decay = ofLerp(0.5f, 0.05f, t); //big decay = dark
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= kH; ++k)
               value += expf(-decay * (k - 1)) * sinf(phase01 * FTWO_PI * k);
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"tanh drive": a sine pushed through a tanh waveshaper, morphing clean sine -> soft
      //saturated square as the drive increases (adds odd harmonics musically)
      auto table = std::make_shared<WavetableFrameSet>("tanh drive");
      const int kFrames = 8;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         float drive = ofLerp(1.0f, 8.0f, t);
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            frame[n] = tanhf(drive * sinf(phase01 * FTWO_PI));
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   {
      //"digital": harmonic stack with fixed pseudo-random phase offsets (golden-angle spread) and a
      //growing harmonic count - gives a gritty, glassy digital character that thickens as you scan
      auto table = std::make_shared<WavetableFrameSet>("digital");
      const int kFrames = 8;
      for (int i = 0; i < kFrames; ++i)
      {
         float t = (float)i / (kFrames - 1);
         int numH = 8 + (int)(t * 40); //8 -> 48 harmonics
         std::vector<float> frame(N, 0.0f);
         for (int n = 0; n < N; ++n)
         {
            float phase01 = (float)n / N;
            float value = 0;
            for (int k = 1; k <= numH; ++k)
            {
               float phaseOffset = fmodf((float)k * 2.39996323f, FTWO_PI); //golden-angle phase spread
               value += (1.0f / k) * sinf(phase01 * FTWO_PI * k + phaseOffset);
            }
            frame[n] = value;
         }
         normalizeFrame(frame);
         table->AddFrame(frame);
      }
      table->FinalizeHarmonicCaches();
      sTables.push_back(table);
   }

   return sTables;
}

//static
float WavetableTables::ReadWarped(const WavetableFrameSet* table, float position, float phase, WavetableWarpType warp, float warpAmount)
{
   if (table == nullptr || table->GetFrameCount() == 0)
      return 0;

   float p = phase / FTWO_PI;
   p -= floorf(p); //wrap to [0,1)

   bool oddOnly = (warp == WavetableWarpType::OddOnly);
   bool evenOnly = (warp == WavetableWarpType::EvenOnly);
   float postFlipPoint = -1; //>=0 means: flip sign of the output once p has passed this point

   if (!oddOnly && !evenOnly)
   {
      switch (warp)
      {
         case WavetableWarpType::BendPlus:
         case WavetableWarpType::BendMinus:
         case WavetableWarpType::BendBoth:
         {
            float amount = warpAmount;
            bool doPlus;
            if (warp == WavetableWarpType::BendBoth)
            {
               doPlus = amount >= 0.5f;
               amount = fabsf(amount - 0.5f) * 2; //0 at center, 1 at either extreme
            }
            else
            {
               doPlus = (warp == WavetableWarpType::BendPlus);
            }
            float exponent = doPlus ? (1.0f + amount * 4) : (1.0f / (1.0f + amount * 4));
            float t = p * 2 - 1;
            float warped = (t < 0 ? -1.0f : 1.0f) * powf(fabsf(t), exponent);
            p = (warped + 1) * 0.5f;
            break;
         }
         case WavetableWarpType::AsymPlus:
         case WavetableWarpType::AsymMinus:
         case WavetableWarpType::AsymBoth:
         {
            float amount = warpAmount;
            bool doPlus;
            if (warp == WavetableWarpType::AsymBoth)
            {
               doPlus = amount >= 0.5f;
               amount = fabsf(amount - 0.5f) * 2;
            }
            else
            {
               doPlus = (warp == WavetableWarpType::AsymPlus);
            }
            float exponent = doPlus ? (1.0f + amount * 4) : (1.0f / (1.0f + amount * 4));
            p = powf(p, exponent);
            break;
         }
         case WavetableWarpType::Flip:
         {
            postFlipPoint = ofClamp(warpAmount, 0, 1);
            break;
         }
         case WavetableWarpType::Mirror:
         {
            if (p < 0.5f)
               p = p * 2;
            else
               p = 1.0f - (p - 0.5f) * 2;
            break;
         }
         case WavetableWarpType::Quantize:
         {
            float steps = ofLerp(128.0f, 2.0f, ofClamp(warpAmount, 0, 1));
            p = floorf(p * steps) / steps;
            break;
         }
         default:
            break;
      }
   }

   p -= floorf(p); //re-wrap in case a warp pushed it outside [0,1)

   //frame blend across the scan position
   int frameCount = table->GetFrameCount();
   float frameIdxF = ofClamp(position, 0, 1) * (frameCount - 1);
   int frameLow = (int)frameIdxF;
   int frameHigh = std::min(frameLow + 1, frameCount - 1);
   float frameBlend = frameIdxF - frameLow;

   const float* dataLow = table->GetFrameData(frameLow, oddOnly, evenOnly);
   const float* dataHigh = table->GetFrameData(frameHigh, oddOnly, evenOnly);

   auto lookup = [](const float* frameData, float p01)
   {
      float idxF = p01 * WavetableFrameSet::kTableSize;
      int i0 = (int)idxF;
      int i1 = (i0 + 1) % WavetableFrameSet::kTableSize;
      float frac = idxF - i0;
      return frameData[i0] + (frameData[i1] - frameData[i0]) * frac;
   };

   float sampleLow = lookup(dataLow, p);
   float sampleHigh = lookup(dataHigh, p);
   float result = sampleLow + (sampleHigh - sampleLow) * frameBlend;

   if (postFlipPoint >= 0 && p > postFlipPoint)
      result = -result;

   return result;
}

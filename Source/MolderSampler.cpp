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
//  MolderSampler.cpp
//  Bespoke
//

#include "MolderSampler.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Scale.h"
#include "FFT.h"
#include "Profiler.h"
#include <algorithm>
#include <cstdlib>

namespace
{
   //clean layout: full-width waveform on top, three control columns below
   const float kMargin = 6;
   const float kWaveX = 6; //waveform spans the full module width
   const float kWaveY = 20;
   const float kWaveH = 96;

   const float kRowY = 128; //top of the three-column control row

   const float kCol1X = 6; //main controls
   const float kCol1W = 150;
   const float kCol2X = 162; //mode-specific params
   const float kCol2W = 150;
   const float kCol3X = 318; //envelope
   const float kCol3W = 146;

   const float kRowStep = 18; //vertical spacing between controls in a column
}

MolderSampler::MolderSampler()
: IAudioProcessor(gBufferSize)
, mWriteBuffer(gBufferSize)
{
   mWriteBuffer.SetNumActiveChannels(2);
   mAdsr.Set(5, 0, 1, 80);
   mSpecTable.resize(kSpecSize, 0.0f);
}

MolderSampler::~MolderSampler()
{
}

void MolderSampler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   //--- column 1: main controls ---
   float y = kRowY;
   mModeSelector = new DropdownList(this, "mode", kCol1X, y, &mMode, kCol1W);
   y += kRowStep;
   mRootSlider = new IntSlider(this, "root", kCol1X, y, kCol1W, 14, &mRootPitch, 24, 96);
   y += kRowStep;
   mGainSlider = new FloatSlider(this, "volume", kCol1X, y, kCol1W, 14, &mGain, 0, 2);
   y += kRowStep;
   mSpeedSlider = new FloatSlider(this, "speed", kCol1X, y, kCol1W, 14, &mSpeed, 0, 2);
   y += kRowStep;
   //GLOBAL play-region behaviour, always visible - every engine obeys the two region markers
   mRegionModeSelector = new DropdownList(this, "region", kCol1X, y, &mRegionMode, kCol1W);

   //--- column 2: mode-specific params (shown/hidden per mode, all anchored to the same origin) ---
   float mx = kCol2X;
   float my = kRowY;
   mSliceModeSelector = new DropdownList(this, "slice by", mx, my, &mSliceMode, kCol2W);
   mNumSlicesSlider = new IntSlider(this, "slices", mx, my + kRowStep, kCol2W, 14, &mNumSlices, 1, 32);
   mOnsetSensitivitySlider = new FloatSlider(this, "sensitivity", mx, my + kRowStep, kCol2W, 14, &mOnsetSensitivity, 0, 1);
   mResliceButton = new ClickButton(this, "reslice", mx, my + kRowStep * 2);
   //quick-division buttons (like SamplePlayer's 4/8/16/32)
   mSlice4 = new ClickButton(this, "4", mx, my + kRowStep * 3);
   mSlice8 = new ClickButton(this, "8", mx + 26, my + kRowStep * 3);
   mSlice16 = new ClickButton(this, "16", mx + 52, my + kRowStep * 3);
   mSlice32 = new ClickButton(this, "32", mx + 84, my + kRowStep * 3);

   mSnapScaleCheckbox = new Checkbox(this, "snap to scale", mx, my, &mSnapScale);

   mGrainPosSlider = new FloatSlider(this, "pos", mx, my, kCol2W, 14, &mGrainPos, 0, 1);
   mGrainSizeSlider = new FloatSlider(this, "grain ms", mx, my + kRowStep, kCol2W, 14, &mGrainSizeMs, 5, 400);
   mGrainSpreadSlider = new FloatSlider(this, "spread", mx, my + kRowStep * 2, kCol2W, 14, &mGrainSpread, 0, 0.5f);
   mGrainDensitySlider = new FloatSlider(this, "density", mx, my + kRowStep * 3, kCol2W, 14, &mGrainDensity, 0, 1);
   mFreezeCheckbox = new Checkbox(this, "freeze", mx, my + kRowStep * 4, &mFreeze);

   mSpecPosSlider = new FloatSlider(this, "spos", mx, my, kCol2W, 14, &mSpecPos, 0, 1);
   mSpecTiltSlider = new FloatSlider(this, "tilt", mx, my + kRowStep, kCol2W, 14, &mSpecTilt, -1, 1);
   mSpecHarmonicsSlider = new FloatSlider(this, "harmonics", mx, my + kRowStep * 2, kCol2W, 14, &mSpecHarmonics, 0, 1);

   //--- column 3: envelope + audio recording ---
   mADSRDisplay = new ADSRDisplay(this, "env", kCol3X, kRowY + 12, kCol3W, 74, &mAdsr);
   mRecordCheckbox = new Checkbox(this, "record", kCol3X, kRowY + 12 + 74 + 6, &mRecording);

   mModeSelector->AddLabel("classic", kMode_Classic);
   mModeSelector->AddLabel("slice", kMode_Slice);
   mModeSelector->AddLabel("repitch", kMode_Repitch);
   mModeSelector->AddLabel("granular", kMode_Granular);
   mModeSelector->AddLabel("spectral", kMode_Spectral);

   mRegionModeSelector->AddLabel("one-shot", 0);
   mRegionModeSelector->AddLabel("loop", 1);
   mRegionModeSelector->AddLabel("ping-pong", 2);
   mRegionModeSelector->AddLabel("reverse", 3);

   mSliceModeSelector->AddLabel("divisions", 0);
   mSliceModeSelector->AddLabel("onsets", 1);
   mSliceModeSelector->AddLabel("manual", 2);
}

int MolderSampler::SourceLen() const
{
   return mHasSample ? mSample.LengthInSamples() : 0;
}

void MolderSampler::RebuildSlices()
{
   mSliceStarts.clear();
   int len = SourceLen();
   if (len <= 0)
      return;

   //everything is confined to the GLOBAL play region [rs, re]
   int rs = (int)ofClamp(mRegionStart * len, 0.0f, (float)len);
   int re = (int)ofClamp(mRegionEnd * len, 0.0f, (float)len);
   if (re <= rs + 1)
   {
      rs = 0;
      re = len;
   }
   const int regionLen = re - rs;

   if (mSliceMode == 2)
   {
      //manual: start with just the region endpoints; the user clicks the waveform to add markers
      mSliceStarts.push_back(rs);
      mSliceStarts.push_back(re);
      return;
   }

   if (mSliceMode == 0)
   {
      //equal divisions across the region
      int n = ofClamp(mNumSlices, 1, 32);
      for (int i = 0; i <= n; ++i)
         mSliceStarts.push_back(rs + (int)((double)i / n * regionLen));
   }
   else
   {
      //onset detection via local adaptive peak-picking on the energy flux (aubio-style).
      //Fixes the old "all slices clump at the start" bug: we normalise the flux, require each
      //onset to be a true LOCAL MAXIMUM that also clears a moving-average threshold, and enforce
      //a generous minimum gap. Sensitivity lowers the threshold delta -> more onsets.
      ChannelBuffer* data = mSample.Data();
      const float* ch0 = data->GetChannel(0);
      const int win = 1024;
      const int hop = 512;

      //energy per hop, then half-wave-rectified difference (flux)
      std::vector<float> energy;
      for (int pos = rs; pos + win < re; pos += hop)
      {
         float e = 0;
         for (int i = 0; i < win; ++i)
            e += ch0[pos + i] * ch0[pos + i];
         energy.push_back(sqrtf(e / win));
      }
      std::vector<float> flux(energy.size(), 0.0f);
      for (int k = 1; k < (int)energy.size(); ++k)
      {
         float f = energy[k] - energy[k - 1];
         flux[k] = f > 0 ? f : 0;
      }
      //normalise flux to 0..1 so the threshold is scale-independent
      float maxFlux = 1e-9f;
      for (float f : flux)
         maxFlux = MAX(maxFlux, f);
      for (float& f : flux)
         f /= maxFlux;

      const int w = 4; //local-max half-window (hops)
      float sens = ofClamp(mOnsetSensitivity, 0.0f, 1.0f);
      float delta = ofLerp(0.28f, 0.02f, sens); //how far above the local average a peak must sit
      int minGapHops = MAX(1, (int)(gSampleRate * 0.09f / hop)); //~90ms between onsets
      int lastK = -minGapHops;

      std::vector<int> onsets;
      onsets.push_back(rs);
      for (int k = w; k < (int)flux.size() - w && (int)onsets.size() < 64; ++k)
      {
         //local mean around k
         float mean = 0;
         for (int j = k - w; j <= k + w; ++j)
            mean += flux[j];
         mean /= (2 * w + 1);
         //must be the strict local maximum in the window
         bool isPeak = true;
         for (int j = k - w; j <= k + w; ++j)
            if (j != k && flux[j] > flux[k])
            {
               isPeak = false;
               break;
            }
         if (isPeak && flux[k] > mean + delta && flux[k] > delta && (k - lastK) >= minGapHops)
         {
            onsets.push_back(rs + k * hop);
            lastK = k;
         }
      }
      if ((int)onsets.size() < 2)
      {
         onsets.clear();
         for (int i = 0; i < 8; ++i)
            onsets.push_back(rs + (int)((double)i / 8 * regionLen));
      }
      for (int o : onsets)
         mSliceStarts.push_back(o);
      mSliceStarts.push_back(re);
   }
}

void MolderSampler::RebuildSpectral()
{
   mSpecDirty = false;
   for (int i = 0; i < kSpecSize; ++i)
      mSpecTable[i] = 0;
   int len = SourceLen();
   if (len <= 0)
      return;

   ChannelBuffer* data = mSample.Data();
   const float* ch0 = data->GetChannel(0);
   //freeze a window from inside the global region
   int regStart = (int)ofClamp(mRegionStart * len, 0.0f, (float)len);
   int regEnd = (int)ofClamp(mRegionEnd * len, 0.0f, (float)len);
   if (regEnd <= regStart + 1)
   {
      regStart = 0;
      regEnd = len;
   }
   int maxStart = MAX(regStart, regEnd - kSpecSize);
   int start = (int)ofClamp(regStart + mSpecPos * (regEnd - regStart), (float)regStart, (float)maxStart);

   std::vector<float> in(kSpecSize, 0.0f);
   for (int i = 0; i < kSpecSize; ++i)
   {
      int idx = start + i;
      float s = (idx < len) ? ch0[idx] : 0.0f;
      float win = 0.5f - 0.5f * cosf(FTWO_PI * i / kSpecSize); //hann
      in[i] = s * win;
   }

   const int nfreq = kSpecSize / 2 + 1;
   std::vector<float> re(nfreq, 0.0f), im(nfreq, 0.0f), out(kSpecSize, 0.0f);
   FFT fft(kSpecSize);
   fft.Forward(in.data(), re.data(), im.data());

   //spectral tilt (brightness) + harmonic boost of upper partials, then resynthesize
   for (int k = 0; k < nfreq; ++k)
   {
      float norm = (float)k / nfreq;
      float tiltGain = powf(k + 1.0f, mSpecTilt); //>1 brightens highs, <1 darkens
      float harmGain = 1.0f + mSpecHarmonics * norm * 3.0f;
      float g = tiltGain * harmGain;
      re[k] *= g;
      im[k] *= g;
   }

   fft.Inverse(re.data(), im.data(), out.data());

   float peak = 0.0001f;
   for (int i = 0; i < kSpecSize; ++i)
      peak = MAX(peak, fabsf(out[i]));
   float scale = 0.9f / peak;
   for (int i = 0; i < kSpecSize; ++i)
      mSpecTable[i] = out[i] * scale;
}

void MolderSampler::PlayNote(NoteMessage note)
{
   if (!mEnabled || !mHasSample)
      return;

   int len = SourceLen();
   if (len <= 0)
      return;

   if (note.velocity > 0)
   {
      if (mMode == kMode_Spectral && mSpecDirty)
         RebuildSpectral();

      //find a free voice, else steal the oldest
      int slot = -1;
      double oldest = 1e18;
      for (int i = 0; i < kNumVoices; ++i)
      {
         if (!mVoices[i].mActive)
         {
            slot = i;
            break;
         }
         if (mVoices[i].mStartTime < oldest)
         {
            oldest = mVoices[i].mStartTime;
            slot = i;
         }
      }

      Voice& v = mVoices[slot];
      v.mActive = true;
      v.mPitch = note.pitch;
      v.mVel = note.velocity / 127.0f;
      v.mStartTime = note.time;
      v.mAdsr.Set(mAdsr);
      v.mAdsr.Start(note.time, 1);
      v.mDir = 1;
      v.mOneShotDone = false;
      v.mPos = 0;
      v.mSpecPhase = 0;

      //global play region in sample coords
      double rs = ofClamp(mRegionStart, 0.0f, 1.0f) * len;
      double re = ofClamp(mRegionEnd, 0.0f, 1.0f) * len;
      if (re <= rs + 1)
      {
         rs = 0;
         re = len;
      }

      if (mMode == kMode_Slice && (int)mSliceStarts.size() >= 2)
      {
         int numSlices = (int)mSliceStarts.size() - 1;
         int sliceIdx = ofClamp(note.pitch - mBaseNote, 0, numSlices - 1);
         v.mRegionStart = mSliceStarts[sliceIdx];
         v.mRegionEnd = mSliceStarts[sliceIdx + 1];
      }
      else
      {
         v.mRegionStart = rs;
         v.mRegionEnd = re;
      }

      //start at the region edge dictated by the region mode (reverse starts at the end)
      if (mRegionMode == 3)
      {
         v.mPos = v.mRegionEnd - 1;
         v.mDir = -1;
      }
      else
      {
         v.mPos = v.mRegionStart;
         v.mDir = 1;
      }

      v.mScrub = rs + ofClamp(mGrainPos, 0.0f, 1.0f) * (re - rs);
      v.mGrainPhase = 1.0; //force a fresh grain on the first sample
      v.mGrainStart = v.mScrub;
   }
   else
   {
      for (int i = 0; i < kNumVoices; ++i)
         if (mVoices[i].mActive && mVoices[i].mPitch == note.pitch)
            mVoices[i].mAdsr.Stop(note.time);
   }
}

void MolderSampler::Process(double time)
{
   PROFILER(MolderSampler);

   SyncBuffers();

   //--- capture incoming audio while armed (runs even before a sample exists) ---------------------
   {
      int inSize = GetBuffer()->BufferSize();
      const float* in0 = GetBuffer()->GetChannel(0);
      float lvl = 0;
      if (in0 != nullptr)
      {
         for (int i = 0; i < inSize; ++i)
            lvl = MAX(lvl, fabsf(in0[i]));
         if (mRecording && !mPendingFinalize)
         {
            //write by index into the pre-allocated buffer - no heap allocation on the audio thread
            const size_t cap = mRecordBuf.size();
            for (int i = 0; i < inSize && mRecordLen < cap; ++i)
               mRecordBuf[mRecordLen++] = in0[i];
            if (mRecordLen >= cap)
               mPendingFinalize = true; //auto-stop; UI thread bakes the sample in Poll()
         }
      }
      mInputLevel = mInputLevel * 0.8f + lvl * 0.2f;
   }
   GetBuffer()->Reset();

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr || !mHasSample)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   int len = SourceLen();
   ChannelBuffer* data = mSample.Data();
   const float* ch0 = data->GetChannel(0);
   float ratio = mSample.GetSampleRateRatio();

   if (mMode == kMode_Spectral && mSpecDirty)
      RebuildSpectral();

   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   if (len <= 0)
      return;

   float grainLen = MAX(4.0f, mGrainSizeMs * 0.001f * gSampleRate);

   //global play region (sample coords) - obeyed by every engine
   double gRegionStart = ofClamp(mRegionStart, 0.0f, 1.0f) * len;
   double gRegionEnd = ofClamp(mRegionEnd, 0.0f, 1.0f) * len;
   if (gRegionEnd <= gRegionStart + 1)
   {
      gRegionStart = 0;
      gRegionEnd = len;
   }
   double gRegionLen = gRegionEnd - gRegionStart;

   for (int vi = 0; vi < kNumVoices; ++vi)
   {
      Voice& v = mVoices[vi];
      if (!v.mActive)
         continue;

      //pitch rate for the pitched modes
      int usePitch = v.mPitch;
      if (mMode == kMode_Repitch && mSnapScale)
         usePitch = TheScale->MakeDiatonic(v.mPitch);
      float rate = powf(2.0f, (usePitch - mRootPitch) / 12.0f) * ratio * mSpeed;
      float sliceRate = ratio * mSpeed; //slices play at original pitch, scaled by the speed knob

      double t = time;
      for (int i = 0; i < bufferSize; ++i)
      {
         if (v.mAdsr.IsDone(t))
         {
            v.mActive = false;
            break;
         }
         float env = v.mAdsr.Value(t);
         float s = 0;

         if (mMode == kMode_Classic || mMode == kMode_Slice || mMode == kMode_Repitch)
         {
            //unified region playhead. bounds are the voice's region (whole play-region for classic/
            //repitch, the individual slice for slice mode - which itself lives inside the region).
            //mRegionMode drives how the playhead behaves at the boundaries, for EVERY engine.
            double lo = v.mRegionStart;
            double hi = v.mRegionEnd;
            double span = MAX(1.0, hi - lo);
            double step = (mMode == kMode_Slice) ? (double)sliceRate : (double)rate;

            s = GetInterpolatedSample(v.mPos, ch0, len);
            v.mPos += step * v.mDir;

            if (mRegionMode == 0) //one-shot
            {
               if (v.mPos >= hi || v.mPos < lo)
               {
                  v.mActive = false;
                  break;
               }
            }
            else if (mRegionMode == 1) //loop forward
            {
               if (v.mPos >= hi)
                  v.mPos -= span;
               if (v.mPos < lo)
                  v.mPos += span;
            }
            else if (mRegionMode == 2) //ping-pong
            {
               if (v.mPos >= hi - 1)
               {
                  v.mDir = -1;
                  v.mPos = hi - 1;
               }
               else if (v.mPos <= lo)
               {
                  v.mDir = 1;
                  v.mPos = lo;
               }
            }
            else //reverse (loop backwards)
            {
               if (v.mPos < lo)
                  v.mPos += span;
               if (v.mPos >= hi)
                  v.mPos -= span;
            }
         }
         else if (mMode == kMode_Granular)
         {
            if (v.mGrainPhase >= 1.0)
            {
               v.mGrainPhase = 0;
               float spreadOff = ofRandom(-1.0f, 1.0f) * mGrainSpread * (float)gRegionLen;
               v.mGrainStart = v.mScrub + spreadOff;
            }
            double readPos = v.mGrainStart + v.mGrainPhase * grainLen * rate;
            //wrap the read position inside the global region
            readPos = gRegionStart + fmod(readPos - gRegionStart, gRegionLen);
            if (readPos < gRegionStart)
               readPos += gRegionLen;
            float win = 0.5f - 0.5f * cosf(FTWO_PI * (float)v.mGrainPhase); //hann
            s = GetInterpolatedSample(readPos, ch0, len) * win;
            v.mGrainPhase += 1.0 / grainLen;
            if (!mFreeze)
            {
               v.mScrub += ratio * (0.2f + mGrainDensity);
               if (v.mScrub >= gRegionEnd)
                  v.mScrub -= gRegionLen;
               if (v.mScrub < gRegionStart)
                  v.mScrub += gRegionLen;
            }
         }
         else //spectral
         {
            float freq = 440.0f * powf(2.0f, (v.mPitch - 69) / 12.0f);
            double inc = (double)kSpecSize * freq / gSampleRate;
            int idx = (int)v.mSpecPhase;
            if (idx < 0)
               idx = 0;
            if (idx >= kSpecSize)
               idx = kSpecSize - 1;
            s = mSpecTable[idx];
            v.mSpecPhase += inc;
            while (v.mSpecPhase >= kSpecSize)
               v.mSpecPhase -= kSpecSize;
         }

         float outv = s * env * v.mVel * mGain * 0.6f;
         mWriteBuffer.GetChannel(0)[i] += outv;
         mWriteBuffer.GetChannel(1)[i] += outv;

         t += gInvSampleRateMs;
      }
   }

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void MolderSampler::SampleDropped(int x, int y, Sample* sample)
{
   if (sample == nullptr || sample->LengthInSamples() <= 0)
      return;
   mSample.CopyFrom(sample);
   mHasSample = true;
   mRegionStart = 0.0f; //reset the global play region to the whole new sample
   mRegionEnd = 1.0f;
   RebuildSlices();
   mSpecDirty = true;
}

void MolderSampler::OnClicked(float x, float y, bool right)
{
   int len = SourceLen();
   float ww = mWidth - kWaveX - 6;
   bool inWave = (len > 0 && x >= kWaveX && x <= kWaveX + ww && y >= kWaveY && y <= kWaveY + kWaveH);

   if (inWave)
   {
      //1) grab a global region marker if the click is near one (works in every mode)
      float startX = kWaveX + ww * ofClamp(mRegionStart, 0.0f, 1.0f);
      float endX = kWaveX + ww * ofClamp(mRegionEnd, 0.0f, 1.0f);
      const float grab = 6.0f;
      if (!right && fabsf(x - startX) <= grab && fabsf(x - startX) <= fabsf(x - endX))
      {
         mDragMarker = 1;
         mDragOffsetX = TheSynth->GetMouseX(GetOwningContainer()) - x;
         return;
      }
      if (!right && fabsf(x - endX) <= grab)
      {
         mDragMarker = 2;
         mDragOffsetX = TheSynth->GetMouseX(GetOwningContainer()) - x;
         return;
      }

      double frac = ofClamp((x - kWaveX) / ww, 0.0f, 1.0f);
      int pos = (int)(frac * len);

      //2) granular: tapping the waveform sets the scrub/granulate position
      if (mMode == kMode_Granular)
      {
         double rs = ofClamp(mRegionStart, 0.0f, 1.0f) * len;
         double re = ofClamp(mRegionEnd, 0.0f, 1.0f) * len;
         if (re <= rs + 1)
         {
            rs = 0;
            re = len;
         }
         mGrainPos = ofClamp((float)((pos - rs) / MAX(1.0, re - rs)), 0.0f, 1.0f);
         return;
      }

      //3) slice mode: click adds a marker, double-click (or right-click) removes the nearest one
      if (mMode == kMode_Slice)
      {
         if ((int)mSliceStarts.size() < 2)
            RebuildSlices();

         double now = gTime;
         bool doubleClick = (now - mLastClickMs < 400.0) && fabsf(x - mLastClickLocalX) < 6.0f;
         mLastClickMs = now;
         mLastClickLocalX = x;

         if (right || doubleClick)
         {
            //remove nearest interior boundary (never the region-edge anchors)
            int bestIdx = -1, bestDist = 1 << 30;
            for (int i = 1; i < (int)mSliceStarts.size() - 1; ++i)
            {
               int d = abs(mSliceStarts[i] - pos);
               if (d < bestDist)
               {
                  bestDist = d;
                  bestIdx = i;
               }
            }
            if (bestIdx > 0)
               mSliceStarts.erase(mSliceStarts.begin() + bestIdx);
         }
         else
         {
            //add a marker, but keep it inside the region and not on top of an existing one
            int rs = mSliceStarts.front();
            int re = mSliceStarts.back();
            pos = (int)ofClamp((float)pos, (float)rs, (float)re);
            int minGap = (int)(gSampleRate * 0.005f);
            bool tooClose = false;
            for (int b : mSliceStarts)
               if (abs(b - pos) < minGap)
               {
                  tooClose = true;
                  break;
               }
            if (!tooClose)
            {
               mSliceStarts.push_back(pos);
               std::sort(mSliceStarts.begin(), mSliceStarts.end());
            }
         }
         return;
      }
   }
   IDrawableModule::OnClicked(x, y, right);
}

void MolderSampler::Poll()
{
   //drag a region marker while the left mouse button is held
   if (mDragMarker != 0)
   {
      if (TheSynth->IsMouseButtonHeld(1))
      {
         float ww = mWidth - kWaveX - 6;
         float localX = TheSynth->GetMouseX(GetOwningContainer()) - mDragOffsetX;
         float frac = ofClamp((localX - kWaveX) / ww, 0.0f, 1.0f);
         if (mDragMarker == 1)
            mRegionStart = MIN(frac, mRegionEnd - 0.01f);
         else
            mRegionEnd = MAX(frac, mRegionStart + 0.01f);
         RebuildSlices();
         mSpecDirty = true;
      }
      else
      {
         mDragMarker = 0;
      }
   }

   //bake a finished recording into the sample on the UI thread (requested by the audio thread)
   if (mPendingFinalize)
   {
      FinalizeRecording();
      mPendingFinalize = false;
   }
}

void MolderSampler::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRecordCheckbox)
   {
      if (mRecording)
      {
         //armed -> allocate the whole 30s capture buffer up front (UI thread), so the audio thread
         //never has to allocate; it just writes into it by index
         mRecordBuf.assign((size_t)(gSampleRate * 30), 0.0f);
         mRecordLen = 0;
         mPendingFinalize = false;
      }
      else
      {
         //disarmed -> bake whatever we captured
         mPendingFinalize = true;
      }
   }
}

void MolderSampler::FinalizeRecording()
{
   int n = (int)mRecordLen; //only the samples actually written, not the full 30s buffer
   if (n <= 0)
      return;
   mSample.Create(n); //1-channel buffer at the engine sample rate (ratio 1)
   float* dst = mSample.Data()->GetChannel(0);
   for (int i = 0; i < n; ++i)
      dst[i] = mRecordBuf[i];
   mSample.SetName("recording");
   mHasSample = true;
   mRegionStart = 0.0f;
   mRegionEnd = 1.0f;
   RebuildSlices();
   mSpecDirty = true;
   mRecordBuf.clear();
   mRecordLen = 0;
}

void MolderSampler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mModeSelector->Draw();
   mRootSlider->Draw();
   mGainSlider->Draw();
   mSpeedSlider->Draw();
   mRegionModeSelector->Draw(); //global region behaviour, always visible
   mADSRDisplay->Draw();

   //show only the active mode's parameter controls
   mSliceModeSelector->SetShowing(mMode == kMode_Slice);
   mNumSlicesSlider->SetShowing(mMode == kMode_Slice && mSliceMode == 0);
   mOnsetSensitivitySlider->SetShowing(mMode == kMode_Slice && mSliceMode == 1);
   mResliceButton->SetShowing(mMode == kMode_Slice);
   mSlice4->SetShowing(mMode == kMode_Slice);
   mSlice8->SetShowing(mMode == kMode_Slice);
   mSlice16->SetShowing(mMode == kMode_Slice);
   mSlice32->SetShowing(mMode == kMode_Slice);
   mSnapScaleCheckbox->SetShowing(mMode == kMode_Repitch);
   mGrainPosSlider->SetShowing(mMode == kMode_Granular);
   mGrainSizeSlider->SetShowing(mMode == kMode_Granular);
   mGrainSpreadSlider->SetShowing(mMode == kMode_Granular);
   mGrainDensitySlider->SetShowing(mMode == kMode_Granular);
   mFreezeCheckbox->SetShowing(mMode == kMode_Granular);
   mSpecPosSlider->SetShowing(mMode == kMode_Spectral);
   mSpecTiltSlider->SetShowing(mMode == kMode_Spectral);
   mSpecHarmonicsSlider->SetShowing(mMode == kMode_Spectral);

   mSliceModeSelector->Draw();
   mNumSlicesSlider->Draw();
   mOnsetSensitivitySlider->Draw();
   mResliceButton->Draw();
   mSlice4->Draw();
   mSlice8->Draw();
   mSlice16->Draw();
   mSlice32->Draw();
   mSnapScaleCheckbox->Draw();
   mGrainPosSlider->Draw();
   mGrainSizeSlider->Draw();
   mGrainSpreadSlider->Draw();
   mGrainDensitySlider->Draw();
   mFreezeCheckbox->Draw();
   mSpecPosSlider->Draw();
   mSpecTiltSlider->Draw();
   mSpecHarmonicsSlider->Draw();

   //record checkbox + input level meter / REC status
   mRecordCheckbox->Draw();
   {
      float mx = kCol3X + 62;
      float my = kRowY + 12 + 74 + 6;
      ofPushStyle();
      //input level meter
      ofFill();
      ofSetColor(40, 44, 52, 200);
      ofRect(mx, my + 2, 44, 9);
      float lvl = ofClamp(mInputLevel * 3.0f, 0.0f, 1.0f);
      ofSetColor(mRecording ? 235 : 120, mRecording ? 70 : 200, mRecording ? 70 : 120, 220);
      ofRect(mx, my + 2, 44 * lvl, 9);
      if (mRecording)
      {
         //blinking REC dot
         float blink = (sinf((float)(gTime * 0.006)) > 0) ? 255 : 90;
         ofSetColor(240, 60, 60, blink);
         ofCircle(mx + 50, my + 6, 4);
      }
      ofPopStyle();
   }

   //waveform
   ofPushStyle();
   ofSetColor(12, 14, 18, 220);
   ofFill();
   ofRect(kWaveX, kWaveY, mWidth - kWaveX - 6, kWaveH);
   int len = SourceLen();
   float ww = mWidth - kWaveX - 6;
   if (mHasSample && len > 0)
   {
      const float* ch0 = mSample.Data()->GetChannel(0);
      //proper peak waveform: for each pixel column, draw a vertical line spanning the min..max of
      //the samples in that column (subsampled so long files stay cheap). This reads as a real
      //waveform silhouette instead of the solid band a one-sample-per-pixel line produced.
      ofSetColor(90, 200, 255);
      ofSetLineWidth(1);
      const float midY = kWaveY + kWaveH * 0.5f;
      int steps = (int)ww;
      for (int i = 0; i < steps; ++i)
      {
         int a = (int)((float)i / steps * len);
         int b = (int)((float)(i + 1) / steps * len);
         if (b <= a)
            b = a + 1;
         if (b > len)
            b = len;
         int stride = MAX(1, (b - a) / 64); //cap work per column
         float mn = 1.0f, mx = -1.0f;
         for (int j = a; j < b; j += stride)
         {
            float s = ch0[j];
            if (s < mn)
               mn = s;
            if (s > mx)
               mx = s;
         }
         float y0 = midY - ofClamp(mx, -1.0f, 1.0f) * kWaveH * 0.46f;
         float y1 = midY - ofClamp(mn, -1.0f, 1.0f) * kWaveH * 0.46f;
         ofLine(kWaveX + i, y0, kWaveX + i, y1 + 0.5f);
      }

      //GLOBAL play region: dim everything outside [start, end] and draw two draggable markers
      {
         float sx = kWaveX + ww * ofClamp(mRegionStart, 0.0f, 1.0f);
         float ex = kWaveX + ww * ofClamp(mRegionEnd, 0.0f, 1.0f);
         ofPushStyle();
         ofFill();
         ofSetColor(0, 0, 0, 130); //darken the un-played regions
         if (sx > kWaveX)
            ofRect(kWaveX, kWaveY, sx - kWaveX, kWaveH);
         if (ex < kWaveX + ww)
            ofRect(ex, kWaveY, (kWaveX + ww) - ex, kWaveH);
         //marker lines + top handles
         ofSetColor(90, 255, 150, 230);
         ofSetLineWidth(2);
         ofLine(sx, kWaveY, sx, kWaveY + kWaveH);
         ofRect(sx, kWaveY, 5, 6);
         ofSetColor(255, 120, 120, 230);
         ofLine(ex, kWaveY, ex, kWaveY + kWaveH);
         ofRect(ex - 5, kWaveY, 5, 6);
         ofPopStyle();
      }

      //slice markers
      if (mMode == kMode_Slice && (int)mSliceStarts.size() >= 2)
      {
         ofSetColor(255, 210, 90, 180);
         for (int b = 0; b < (int)mSliceStarts.size(); ++b)
         {
            float sx = kWaveX + ww * (float)mSliceStarts[b] / len;
            ofLine(sx, kWaveY, sx, kWaveY + kWaveH);
         }
      }

      //spectral analysis window (what the FFT is frozen from)
      if (mMode == kMode_Spectral)
      {
         float rs = ofClamp(mRegionStart, 0.0f, 1.0f) * len;
         float re = ofClamp(mRegionEnd, 0.0f, 1.0f) * len;
         if (re <= rs + 1)
         {
            rs = 0;
            re = len;
         }
         float maxStart = MAX(rs, re - kSpecSize);
         float startAbs = ofClamp(rs + mSpecPos * (re - rs), rs, maxStart);
         float startFrac = startAbs / len;
         float wWidth = ww * (float)kSpecSize / len;
         ofPushStyle();
         ofFill();
         ofSetColor(255, 200, 80, 55);
         ofRect(kWaveX + ww * startFrac, kWaveY, MAX(2.0f, wWidth), kWaveH);
         ofPopStyle();
      }

      //live playheads / scrub markers for every sounding voice - thin & crisp
      for (int vi = 0; vi < kNumVoices; ++vi)
      {
         const Voice& v = mVoices[vi];
         if (!v.mActive)
            continue;
         if (mMode == kMode_Granular)
         {
            //show the grain scrub position + a translucent band the width of one grain
            float gx = kWaveX + ww * (float)v.mScrub / len;
            float grainLenS = MAX(4.0f, mGrainSizeMs * 0.001f * gSampleRate);
            float gw = ww * grainLenS / len;
            ofPushStyle();
            ofFill();
            ofSetColor(120, 255, 160, 55);
            ofRect(kWaveX + ww * (float)v.mGrainStart / len, kWaveY, MAX(2.0f, gw), kWaveH);
            ofPopStyle();
            ofSetLineWidth(1.0f);
            ofSetColor(120, 255, 160, 200);
            ofLine(gx, kWaveY, gx, kWaveY + kWaveH);
         }
         else if (mMode != kMode_Spectral)
         {
            //hairline playhead with a small triangular head so it stays legible without being thick
            float px = kWaveX + ww * (float)v.mPos / len;
            ofSetLineWidth(1.0f);
            ofSetColor(255, 255, 255, 190);
            ofLine(px, kWaveY, px, kWaveY + kWaveH);
            ofFill();
            ofSetColor(255, 255, 255, 235);
            ofTriangle(px - 3, kWaveY, px + 3, kWaveY, px, kWaveY + 5);
         }
      }
   }
   else
   {
      ofSetColor(180, 180, 180, 120);
      DrawTextNormal("drop a sample here", kWaveX + 8, kWaveY + kWaveH * 0.5f, 12);
   }
   ofNoFill();
   ofSetColor(180, 180, 180, 90);
   ofRect(kWaveX, kWaveY, ww, kWaveH);
   ofPopStyle();

   if (mHasSample)
      DrawTextNormal(mSample.Name(), kWaveX, kWaveY - 3, 11);
}

void MolderSampler::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mSpecPosSlider || slider == mSpecTiltSlider || slider == mSpecHarmonicsSlider)
      mSpecDirty = true;
   if (slider == mOnsetSensitivitySlider && mSliceMode == 1)
      RebuildSlices();
}

void MolderSampler::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumSlicesSlider)
      RebuildSlices();
}

void MolderSampler::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mSliceModeSelector)
      RebuildSlices();
}

void MolderSampler::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResliceButton)
      RebuildSlices();

   //quick-division buttons: switch to equal divisions and slice into 4/8/16/32
   int quick = 0;
   if (button == mSlice4)
      quick = 4;
   else if (button == mSlice8)
      quick = 8;
   else if (button == mSlice16)
      quick = 16;
   else if (button == mSlice32)
      quick = 32;
   if (quick > 0)
   {
      mSliceMode = 0;
      mNumSlices = quick;
      RebuildSlices();
   }
}

void MolderSampler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void MolderSampler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

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
//  Razor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/3/12.
//
//

#include "Razor.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"

#include <cstring>

namespace
{
   double sineBuffer[514] = { 0, 0.012268, 0.024536, 0.036804, 0.049042, 0.06131, 0.073547, 0.085785, 0.097992, 0.1102, 0.12241, 0.13455, 0.1467, 0.15884, 0.17093, 0.18301, 0.19507, 0.20709, 0.21909, 0.23105, 0.24295, 0.25485, 0.26669, 0.2785, 0.29025, 0.30197, 0.31366, 0.32529, 0.33685, 0.34839, 0.35986, 0.37128, 0.38266, 0.39395, 0.40521, 0.41641, 0.42752, 0.4386, 0.44958, 0.46051, 0.47137, 0.48215, 0.49286, 0.50351, 0.51407, 0.52457, 0.53497, 0.54529, 0.55554, 0.5657, 0.57578, 0.58575, 0.59567, 0.60547, 0.6152, 0.62482, 0.63437, 0.6438, 0.65314, 0.66238, 0.67151, 0.68057, 0.68951, 0.69833, 0.70706, 0.7157, 0.72421, 0.7326, 0.74091, 0.74908, 0.75717, 0.76514, 0.77298, 0.7807, 0.7883, 0.79581, 0.80316, 0.81042, 0.81754, 0.82455, 0.83142, 0.8382, 0.84482, 0.85132, 0.8577, 0.86392, 0.87006, 0.87604, 0.88187, 0.8876, 0.89319, 0.89862, 0.90396, 0.90912, 0.91415, 0.91907, 0.92383, 0.92847, 0.93295, 0.93729, 0.9415, 0.94556, 0.94949, 0.95325, 0.95691, 0.96039, 0.96375, 0.96692, 0.97, 0.9729, 0.97565, 0.97827, 0.98074, 0.98306, 0.98523, 0.98724, 0.98914, 0.99084, 0.99243, 0.99387, 0.99515, 0.99628, 0.99725, 0.99808, 0.99875, 0.99927, 0.99966, 0.99988, 0.99997, 0.99988, 0.99966, 0.99927, 0.99875, 0.99808, 0.99725, 0.99628, 0.99515, 0.99387, 0.99243, 0.99084, 0.98914, 0.98724, 0.98523, 0.98306, 0.98074, 0.97827, 0.97565, 0.9729, 0.97, 0.96692, 0.96375, 0.96039, 0.95691, 0.95325, 0.94949, 0.94556, 0.9415, 0.93729, 0.93295, 0.92847, 0.92383, 0.91907, 0.91415, 0.90912, 0.90396, 0.89862, 0.89319, 0.8876, 0.88187, 0.87604, 0.87006, 0.86392, 0.8577, 0.85132, 0.84482, 0.8382, 0.83142, 0.82455, 0.81754, 0.81042, 0.80316, 0.79581, 0.7883, 0.7807, 0.77298, 0.76514, 0.75717, 0.74908, 0.74091, 0.7326, 0.72421, 0.7157, 0.70706, 0.69833, 0.68951, 0.68057, 0.67151, 0.66238, 0.65314, 0.6438, 0.63437, 0.62482, 0.6152, 0.60547, 0.59567, 0.58575, 0.57578, 0.5657, 0.55554, 0.54529, 0.53497, 0.52457, 0.51407, 0.50351, 0.49286, 0.48215, 0.47137, 0.46051, 0.44958, 0.4386, 0.42752, 0.41641, 0.40521, 0.39395, 0.38266, 0.37128, 0.35986, 0.34839, 0.33685, 0.32529, 0.31366, 0.30197, 0.29025, 0.2785, 0.26669, 0.25485, 0.24295, 0.23105, 0.21909, 0.20709, 0.19507, 0.18301, 0.17093, 0.15884, 0.1467, 0.13455, 0.12241, 0.1102, 0.097992, 0.085785, 0.073547, 0.06131, 0.049042, 0.036804, 0.024536, 0.012268, 0, -0.012268, -0.024536, -0.036804, -0.049042, -0.06131, -0.073547, -0.085785, -0.097992, -0.1102, -0.12241, -0.13455, -0.1467, -0.15884, -0.17093, -0.18301, -0.19507, -0.20709, -0.21909, -0.23105, -0.24295, -0.25485, -0.26669, -0.2785, -0.29025, -0.30197, -0.31366, -0.32529, -0.33685, -0.34839, -0.35986, -0.37128, -0.38266, -0.39395, -0.40521, -0.41641, -0.42752, -0.4386, -0.44958, -0.46051, -0.47137, -0.48215, -0.49286, -0.50351, -0.51407, -0.52457, -0.53497, -0.54529, -0.55554, -0.5657, -0.57578, -0.58575, -0.59567, -0.60547, -0.6152, -0.62482, -0.63437, -0.6438, -0.65314, -0.66238, -0.67151, -0.68057, -0.68951, -0.69833, -0.70706, -0.7157, -0.72421, -0.7326, -0.74091, -0.74908, -0.75717, -0.76514, -0.77298, -0.7807, -0.7883, -0.79581, -0.80316, -0.81042, -0.81754, -0.82455, -0.83142, -0.8382, -0.84482, -0.85132, -0.8577, -0.86392, -0.87006, -0.87604, -0.88187, -0.8876, -0.89319, -0.89862, -0.90396, -0.90912, -0.91415, -0.91907, -0.92383, -0.92847, -0.93295, -0.93729, -0.9415, -0.94556, -0.94949, -0.95325, -0.95691, -0.96039, -0.96375, -0.96692, -0.97, -0.9729, -0.97565, -0.97827, -0.98074, -0.98306, -0.98523, -0.98724, -0.98914, -0.99084, -0.99243, -0.99387, -0.99515, -0.99628, -0.99725, -0.99808, -0.99875, -0.99927, -0.99966, -0.99988, -0.99997, -0.99988, -0.99966, -0.99927, -0.99875, -0.99808, -0.99725, -0.99628, -0.99515, -0.99387, -0.99243, -0.99084, -0.98914, -0.98724, -0.98523, -0.98306, -0.98074, -0.97827, -0.97565, -0.9729, -0.97, -0.96692, -0.96375, -0.96039, -0.95691, -0.95325, -0.94949, -0.94556, -0.9415, -0.93729, -0.93295, -0.92847, -0.92383, -0.91907, -0.91415, -0.90912, -0.90396, -0.89862, -0.89319, -0.8876, -0.88187, -0.87604, -0.87006, -0.86392, -0.8577, -0.85132, -0.84482, -0.8382, -0.83142, -0.82455, -0.81754, -0.81042, -0.80316, -0.79581, -0.7883, -0.7807, -0.77298, -0.76514, -0.75717, -0.74908, -0.74091, -0.7326, -0.72421, -0.7157, -0.70706, -0.69833, -0.68951, -0.68057, -0.67151, -0.66238, -0.65314, -0.6438, -0.63437, -0.62482, -0.6152, -0.60547, -0.59567, -0.58575, -0.57578, -0.5657, -0.55554, -0.54529, -0.53497, -0.52457, -0.51407, -0.50351, -0.49286, -0.48215, -0.47137, -0.46051, -0.44958, -0.4386, -0.42752, -0.41641, -0.40521, -0.39395, -0.38266, -0.37128, -0.35986, -0.34839, -0.33685, -0.32529, -0.31366, -0.30197, -0.29025, -0.2785, -0.26669, -0.25485, -0.24295, -0.23105, -0.21909, -0.20709, -0.19507, -0.18301, -0.17093, -0.15884, -0.1467, -0.13455, -0.12241, -0.1102, -0.097992, -0.085785, -0.073547, -0.06131, -0.049042, -0.036804, -0.024536, -0.012268, 0, 0.012268 };
}

Razor::Razor()
{
   std::memset(mAmp, 0, sizeof(float) * NUM_PARTIALS);
   std::memset(mPeakHistory, 0, sizeof(float) * (VIZ_WIDTH + 1) * RAZOR_HISTORY);
   std::memset(mPhases, 0, sizeof(float) * NUM_PARTIALS);

   for (int i = 0; i < NUM_PARTIALS; ++i)
      mDetune[i] = 1;
}

void Razor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mNumPartialsSlider = new IntSlider(this, "partials", 5, 40, 320, 15, &mUseNumPartials, 1, NUM_PARTIALS);
   mBumpAmpSlider = new FloatSlider(this, "bump freq 1", 5, 60, 320, 15, &mBumps[0].mFreq, 0, 10000);
   mBumpAmpAmtSlider = new FloatSlider(this, "amt 1", 5, 80, 320, 15, &mBumps[0].mAmt, -1, 1);
   mBumpAmpDecaySlider = new FloatSlider(this, "decay 1", 5, 100, 320, 15, &mBumps[0].mDecay, 0.00001f, .01f, 4);
   mBumpAmpSlider2 = new FloatSlider(this, "bump freq 2", 330, 60, 320, 15, &mBumps[1].mFreq, 0, 10000);
   mBumpAmpAmtSlider2 = new FloatSlider(this, "amt 2", 330, 80, 320, 15, &mBumps[1].mAmt, -1, 1);
   mBumpAmpDecaySlider2 = new FloatSlider(this, "decay 2", 330, 100, 320, 15, &mBumps[1].mDecay, 0.00001f, .01f, 4);
   mBumpAmpSlider3 = new FloatSlider(this, "bump freq 3", 660, 60, 320, 15, &mBumps[2].mFreq, 0, 10000);
   mBumpAmpAmtSlider3 = new FloatSlider(this, "amt 3", 660, 80, 320, 15, &mBumps[2].mAmt, -1, 1);
   mBumpAmpDecaySlider3 = new FloatSlider(this, "decay 3", 660, 100, 320, 15, &mBumps[2].mDecay, 0.00001f, .01f, 4);
   mASlider = new FloatSlider(this, "A", 450, 342, 80, 15, &mA, 1, 1000);
   mDSlider = new FloatSlider(this, "D", 450, 358, 80, 15, &mD, 1, 1000);
   mSSlider = new FloatSlider(this, "S", 450, 374, 80, 15, &mS, 0, 1);
   mRSlider = new FloatSlider(this, "R", 450, 390, 80, 15, &mR, 1, 1000);
   mHarmonicSelectorSlider = new IntSlider(this, "harmonics", 5, 120, 160, 15, &mHarmonicSelector, -1, 10);
   mPowFalloffSlider = new FloatSlider(this, "pow falloff", 170, 120, 160, 15, &mPowFalloff, .1f, 2);
   mNegHarmonicsSlider = new IntSlider(this, "neg harmonics", 335, 120, 160, 15, &mNegHarmonics, 1, 10);
   mHarshnessCutSlider = new FloatSlider(this, "harshness cut", 500, 120, 160, 15, &mHarshnessCut, 0, 20000);
   mManualControlCheckbox = new Checkbox(this, "manual control", 4, 145, &mManualControl);

   for (int i = 0; i < NUM_AMP_SLIDERS; ++i)
   {
      mAmpSliders[i] = new FloatSlider(this, ("amp" + ofToString(i)).c_str(), 4, 160 + i * 16, 200, 15, &mAmp[i], -1, 1);
      mDetuneSliders[i] = new FloatSlider(this, ("detune" + ofToString(i)).c_str(), 210, 160 + i * 16, 200, 15, &mDetune[i], .98f, 1.02f);
   }

   mResetDetuneButton = new ClickButton(this, "reset detune", 210, 145);

   mASlider->SetMode(FloatSlider::kSquare);
   mDSlider->SetMode(FloatSlider::kSquare);
   mRSlider->SetMode(FloatSlider::kSquare);
}

Razor::~Razor()
{
}

void Razor::Process(double time)
{
   PROFILER(Razor);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   if (!mManualControl)
      CalcAmp();

   for (int i = 0; i < bufferSize; ++i)
   {
      float freq = TheScale->PitchToFreq(mPitch + (mPitchBend ? mPitchBend->GetValue(i) : 0));

      int oscNyquistLimitIdx = int(gNyquistLimit / freq);

      float write = 0;
      for (int j = 0; j < mUseNumPartials && j < oscNyquistLimitIdx; ++j)
      {
         float phaseInc = 512. / (gSampleRate / (freq)) * (j + 1) * mDetune[j];
         mPhases[j] += phaseInc;
         while (mPhases[j] >= 512)
         {
            mPhases[j] -= 512;
         }

         float sample = SinSample(mPhases[j]) * mAdsr[j].Value(time) * mAmp[j] * mVol;

         write += sample;
      }

      GetVizBuffer()->Write(write, 0);

      out[i] += write;

      time += gInvSampleRateMs;
   }
}

void Razor::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (note.velocity > 0)
   {
      float amount = note.velocity / 127.0f;

      mPitch = note.pitch;
      for (int i = 1; i <= NUM_PARTIALS; ++i)
      {
         mAdsr[i - 1].Start(note.time, amount,
                            mA,
                            mD,
                            mS,
                            mR);
      }

      mPitchBend = note.modulation.pitchBend;
      mModWheel = note.modulation.modWheel;
      mPressure = note.modulation.pressure;
   }
   else if (mPitch == note.pitch)
   {
      for (int i = 0; i < NUM_PARTIALS; ++i)
         mAdsr[i].Stop(note.time);
   }
}

void Razor::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mEnabled)
   {
      DrawViz();
      mNumPartialsSlider->Draw();
      mBumpAmpSlider->Draw();
      mBumpAmpAmtSlider->Draw();
      mBumpAmpDecaySlider->Draw();
      mBumpAmpSlider2->Draw();
      mBumpAmpAmtSlider2->Draw();
      mBumpAmpDecaySlider2->Draw();
      mBumpAmpSlider3->Draw();
      mBumpAmpAmtSlider3->Draw();
      mBumpAmpDecaySlider3->Draw();
      mHarmonicSelectorSlider->Draw();
      mPowFalloffSlider->Draw();
      mNegHarmonicsSlider->Draw();
      mHarshnessCutSlider->Draw();
      mASlider->Draw();
      mDSlider->Draw();
      mSSlider->Draw();
      mRSlider->Draw();

      mManualControlCheckbox->Draw();
      for (int i = 0; i < NUM_AMP_SLIDERS; ++i)
      {
         mAmpSliders[i]->Draw();
         mDetuneSliders[i]->Draw();
      }
      mResetDetuneButton->Draw();
   }
}

void Razor::DrawViz()
{
   ofPushStyle();

   int zeroHeight = 240;
   float baseFreq = TheScale->PitchToFreq(mPitch);
   int oscNyquistLimitIdx = int(gNyquistLimit / baseFreq);

   for (int i = 1; i < RAZOR_HISTORY - 1; ++i)
   {
      float age = 1 - float(i) / RAZOR_HISTORY;
      ofSetColor(0, 200 * age, 255 * age);
      for (int x = 0; x < VIZ_WIDTH; ++x)
      {
         int intHeight = mPeakHistory[(i + mHistoryPtr) % RAZOR_HISTORY][x];
         int intHeightNext = mPeakHistory[(i + mHistoryPtr + 1) % RAZOR_HISTORY][x];
         if (intHeight != 0)
         {
            int xpos = 10 + x + i;
            int ypos = zeroHeight - intHeight - i;
            int xposNext = xpos;
            int yposNext = ypos;
            if (intHeightNext != 0)
            {
               xposNext = 10 + x + i + 1;
               yposNext = zeroHeight - intHeightNext - i + 1;
            }
            if (xpos < 1020)
               ofLine(xpos, ypos, xposNext, yposNext);
         }
      }
   }

   std::memset(mPeakHistory[mHistoryPtr], 0, sizeof(float) * VIZ_WIDTH);
   for (int i = 1; i <= mUseNumPartials && i <= oscNyquistLimitIdx; ++i)
   {
      float height = mAdsr[i - 1].Value(gTime) * mAmp[i - 1];
      int intHeight = int(height * 100.0f);
      if (intHeight == 0)
      {
         if (height > 0)
            intHeight = 1;
         if (height < 0)
            intHeight = -1;
      }
      if (intHeight < 0)
      {
         ofSetColor(255, 0, 0);
         intHeight *= -1;
      }
      else
      {
         ofSetColor(255, 255, 255);
      }
      float freq = baseFreq * i;
      int x = int(ofMap(log2(freq), 4, log2(gNyquistLimit), 0, VIZ_WIDTH, true));
      //int x = int(ofMap(freq,0,gNyquistLimit,0,VIZ_WIDTH,true));
      ofLine(10 + x, zeroHeight, 10 + x, zeroHeight - intHeight);
      mPeakHistory[mHistoryPtr][x] = intHeight;
   }

   mHistoryPtr = (mHistoryPtr - 1 + RAZOR_HISTORY) % RAZOR_HISTORY;
   ofPopStyle();
}

float Razor::SinSample(float phase)
{
   int intPhase = int(phase) % 512;
   float remainder = phase - int(phase);
   return ((1 - remainder) * sineBuffer[intPhase] + remainder * sineBuffer[1 + intPhase]);
}

bool IsPrime(int n)
{
   if (n == 1)
      return true;
   if (n % 2 == 0)
      return (n == 2);
   if (n % 3 == 0)
      return (n == 3);
   int m = sqrt(n);
   for (int i = 5; i <= m; i += 6)
   {
      if (n % i == 0)
         return false;
      if (n % (i + 2) == 0)
         return false;
   }
   return true;
}

bool IsPow2(int n)
{
   while (n)
   {
      if (n == 1)
         return true;
      if (n % 2 && n != 1)
         return false;
      n >>= 1;
   }
   return false;
}

void Razor::CalcAmp()
{
   float baseFreq = TheScale->PitchToFreq(mPitch);
   int oscNyquistLimitIdx = int(gNyquistLimit / baseFreq);

   std::memset(mAmp, 0, sizeof(float) * NUM_PARTIALS);
   for (int i = 1; i <= mUseNumPartials && i <= oscNyquistLimitIdx; ++i)
   {
      if ((mHarmonicSelector == 0 && IsPrime(i)) ||
          (mHarmonicSelector == -1 && IsPow2(i)) ||
          mHarmonicSelector == 1 ||
          (mHarmonicSelector > 0 && i % mHarmonicSelector == 1))
      {
         float freq = baseFreq * i;

         mAmp[i - 1] = 1.0f / powf(i, mPowFalloff);

         for (int j = 0; j < NUM_BUMPS; ++j)
         {
            float freqDist = fabs(mBumps[j].mFreq - freq);
            float dist = PI / 2 - freqDist * mBumps[j].mDecay;
            float bumpAmt = mBumps[j].mAmt * (MIN(1, (tanh(dist) + 1) / 2)); // * ofRandom(1);
            mAmp[i - 1] += bumpAmt;
         }

         if (mNegHarmonics > 0 && i % mNegHarmonics == 1)
            mAmp[i - 1] *= -1;

         if (mHarshnessCut > 0)
         {
            float cutPoint = gNyquistLimit - mHarshnessCut;
            if (freq > cutPoint)
               mAmp[i - 1] *= 1 - ((freq - cutPoint) / mHarshnessCut);
         }
      }
   }
}

void Razor::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void Razor::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      SetEnabled(mEnabled);
   if (checkbox == mManualControlCheckbox)
   {
      if (mManualControl)
         mUseNumPartials = NUM_AMP_SLIDERS;
   }
}

void Razor::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Razor::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumPartialsSlider)
   {
      std::memset(mPhases, 0, sizeof(float) * NUM_PARTIALS);
   }
}

void Razor::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetDetuneButton)
   {
      for (int i = 0; i < NUM_PARTIALS; ++i)
         mDetune[i] = 1;
   }
}

void Razor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Razor::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

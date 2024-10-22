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
//  Compressor.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "Compressor.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "UIControlMacros.h"

namespace
{
   static double lin2dB(double lin)
   {
      static const double LOG_2_DB = 8.6858896380650365530225783783321; // 20 / ln( 10 )
      return log(lin) * LOG_2_DB;
   }

   static double dB2lin(double dB)
   {
      static const double DB_2_LOG = 0.11512925464970228420089957273422; // ln( 10 ) / 20
      return exp(dB * DB_2_LOG);
   }

   const float kMaxLookaheadMs = 50;
}

Compressor::Compressor()
: mDelayBuffer(kMaxLookaheadMs * gSampleRateMs)
{
}

void Compressor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mMixSlider, "mix", &mMix, 0, 1);
   FLOATSLIDER(mDriveSlider, "drive", &mDrive, .01f, 2);
   FLOATSLIDER(mThresholdSlider, "threshold", &mThreshold, -70, 0);
   FLOATSLIDER(mRatioSlider, "ratio", &mRatio, 1, 40);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER(mAttackSlider, "attack", &mAttack, .1f, kMaxLookaheadMs);
   FLOATSLIDER(mReleaseSlider, "release", &mRelease, .1f, 500);
   FLOATSLIDER(mLookaheadSlider, "lookahead", &mLookahead, 0, kMaxLookaheadMs);
   FLOATSLIDER(mOutputAdjustSlider, "output", &mOutputAdjust, 0, 2);
   ENDUIBLOCK(mWidth, mHeight);

   mRatioSlider->SetMode(FloatSlider::kSquare);
   mAttackSlider->SetMode(FloatSlider::kSquare);
   mReleaseSlider->SetMode(FloatSlider::kSquare);
   mLookaheadSlider->SetMode(FloatSlider::kSquare);
   mOutputAdjustSlider->SetMode(FloatSlider::kSquare);

   mEnv.setAttack(mAttack);
   mEnv.setRelease(mRelease);
}

void Compressor::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(Compressor);

   if (!mEnabled)
      return;

   int bufferSize = buffer->BufferSize();
   mDelayBuffer.SetNumChannels(buffer->NumActiveChannels());

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);

      // create sidechain

      float input = 0;
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         input = MAX(input, fabsf(buffer->GetChannel(ch)[i]));
      input *= mDrive;

      /* if desired, one could use another EnvelopeDetector to smooth
       * the rectified signal.
       */

      // convert key to dB
      input += DC_OFFSET; // add DC offset to avoid log( 0 )
      mCurrentInputDb = lin2dB(input); // convert linear -> dB

      // threshold
      double overdB = mCurrentInputDb - mThreshold; // delta over threshold
      if (overdB < 0.0)
         overdB = 0.0;

      // attack/release

      overdB += DC_OFFSET; // add DC offset to avoid denormal
      mEnv.run(overdB, envdB_); // run attack/release envelope
      overdB = envdB_ - DC_OFFSET; // subtract DC offset

      /* REGARDING THE DC OFFSET: In this case, since the offset is added before
       * the attack/release processes, the envelope will never fall below the offset,
       * thereby avoiding denormals. However, to prevent the offset from causing
       * constant gain reduction, we must subtract it from the envelope, yielding
       * a minimum value of 0dB.
       */

      double invRatio = 1 / mRatio;

      // transfer function
      double reduction = overdB * (invRatio - 1.0); // gain reduction (dB)
      double makeup = (-mThreshold * .5) * (1.0 - invRatio);
      mOutputGain = ofLerp(1, dB2lin(reduction + makeup) * mDrive * mOutputAdjust, mMix);

      // output gain
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         mDelayBuffer.Write(buffer->GetChannel(ch)[i], ch);
         buffer->GetChannel(ch)[i] = mDelayBuffer.GetSample(ofClamp(int(mLookahead * gSampleRateMs) + 1, 1, mDelayBuffer.Size() - 1), ch) * mOutputGain; // apply gain reduction to input
      }
   }
}

void Compressor::DrawModule()
{
   mMixSlider->Draw();
   mDriveSlider->Draw();
   mThresholdSlider->Draw();
   mRatioSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();
   mLookaheadSlider->Draw();
   mOutputAdjustSlider->Draw();

   ofPushStyle();
   ofSetColor(0, 255, 0, gModuleDrawAlpha);
   float x, y, w, h;

   mThresholdSlider->GetPosition(x, y, K(local));
   mThresholdSlider->GetDimensions(w, h);
   float currentInputX = ofMap(mCurrentInputDb, mThresholdSlider->GetMin(), mThresholdSlider->GetMax(), x, x + w, K(clamp));
   ofLine(currentInputX, y, currentInputX, y + h);

   mRatioSlider->GetPosition(x, y, K(local));
   mRatioSlider->GetDimensions(w, h);
   float outputNormalized = ofClamp(mOutputGain / 10, 0, 1);
   float currentOutputX = ofLerp(x, x + w, sqrtf(outputNormalized));
   ofLine(currentOutputX, y, currentOutputX, y + h);

   ofPopStyle();
}

void Compressor::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      envdB_ = DC_OFFSET; //reset state
}

void Compressor::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mAttackSlider)
      mEnv.setAttack(MAX(.1f, mAttack));
   if (slider == mReleaseSlider)
      mEnv.setRelease(MAX(.1f, mRelease));
}

//-------------------------------------------------------------
// envelope detector
//-------------------------------------------------------------
EnvelopeDetector::EnvelopeDetector(double ms)
{
   assert(ms > 0.0);
   ms_ = ms;
   setCoef();
}

//-------------------------------------------------------------
void EnvelopeDetector::setTc(double ms)
{
   assert(ms > 0.0);
   ms_ = ms;
   setCoef();
}

//-------------------------------------------------------------
void EnvelopeDetector::setCoef()
{
   coef_ = exp(-1000.0 / (ms_ * gSampleRate));
}

//-------------------------------------------------------------
// attack/release envelope
//-------------------------------------------------------------
AttRelEnvelope::AttRelEnvelope(double att_ms, double rel_ms)
: att_(att_ms)
, rel_(rel_ms)
{
}

//-------------------------------------------------------------
void AttRelEnvelope::setAttack(double ms)
{
   att_.setTc(ms);
}

//-------------------------------------------------------------
void AttRelEnvelope::setRelease(double ms)
{
   rel_.setTc(ms);
}

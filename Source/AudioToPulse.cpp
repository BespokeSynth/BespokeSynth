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
/*
  ==============================================================================

    AudioToPulse.cpp
    Created: 31 Mar 2021 10:18:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioToPulse.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Profiler.h"
#include "UIControlMacros.h"

AudioToPulse::AudioToPulse()
: IAudioProcessor(gBufferSize)
{
}

AudioToPulse::~AudioToPulse()
{
}

void AudioToPulse::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mThresholdSlider, "threshold", &mThreshold, 0, 1);
   FLOATSLIDER(mReleaseSlider, "release", &mRelease, .01f, 1000);
   ENDUIBLOCK(mWidth, mHeight);

   mThresholdSlider->SetMode(FloatSlider::kSquare);
   mReleaseSlider->SetMode(FloatSlider::kSquare);

   //update mReleaseFactor
   FloatSliderUpdated(mReleaseSlider, 0, gTime);

   GetPatchCableSource()->SetConnectionType(kConnectionType_Pulse);
}

void AudioToPulse::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mThresholdSlider->Draw();
   mReleaseSlider->Draw();

   ofPushStyle();
   ofFill();
   ofSetColor(0, 255, 0, gModuleDrawAlpha * .4f);
   ofRectangle rect = mThresholdSlider->GetRect(true);
   rect.width *= ofClamp(sqrtf(mPeak), 0, 1);
   rect.height *= .5f;
   ofRect(rect);
   ofSetColor(255, 0, 0, gModuleDrawAlpha * .4f);
   rect = mThresholdSlider->GetRect(true);
   rect.width *= ofClamp(mEnvelope, 0, 1);
   rect.height *= .5f;
   rect.y += rect.height;
   ofRect(rect);
   ofPopStyle();
}

void AudioToPulse::Process(double time)
{
   PROFILER(AudioToPulse);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   const float kAttackTimeMs = 1;

   assert(GetBuffer()->BufferSize());
   Clear(gWorkBuffer, gBufferSize);
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      Add(gWorkBuffer, GetBuffer()->GetChannel(ch), gBufferSize);
   Mult(gWorkBuffer, 1.0f / GetBuffer()->NumActiveChannels(), gBufferSize);
   for (int i = 0; i < gBufferSize; ++i)
   {
      const float decayTime = .01f;
      float scalar = powf(0.5f, 1.0f / (decayTime * gSampleRate));
      float input = fabsf(gWorkBuffer[i]);

      if (input >= mPeak)
      {
         /* When we hit a peak, ride the peak to the top. */
         mPeak = input;
      }
      else
      {
         /* Exponential decay of output when signal is low. */
         mPeak = mPeak * scalar;
         if (mPeak < FLT_EPSILON)
            mPeak = 0.0;
      }

      float oldEnvelope = mEnvelope;
      if (mPeak >= mThreshold && mEnvelope < 1)
         mEnvelope = MIN(1, mEnvelope + gInvSampleRateMs / kAttackTimeMs);
      if (mPeak < mThreshold && mEnvelope > 0)
         mEnvelope = MAX(0, mEnvelope - gInvSampleRateMs / mRelease);

      if (mEnvelope >= 0.01f && oldEnvelope < 0.01f)
         DispatchPulse(GetPatchCableSource(), time + i * gInvSampleRateMs, 1, 0);
   }

   GetBuffer()->Reset();
}

void AudioToPulse::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mReleaseSlider)
      mReleaseFactor = powf(.01f, 1.0f / (mRelease * gSampleRateMs));
}

void AudioToPulse::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AudioToPulse::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void AudioToPulse::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

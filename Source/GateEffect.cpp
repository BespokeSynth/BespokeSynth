//
//  GateEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/19/13.
//
//

#include "GateEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

GateEffect::GateEffect()
: mThreshold(.1f)
, mAttackTime(1)
, mReleaseTime(1)
, mThresholdSlider(nullptr)
, mAttackSlider(nullptr)
, mReleaseSlider(nullptr)
, mEnvelope(0)
, mPeak(0)
{
}

void GateEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mThresholdSlider = new FloatSlider(this,"threshold",5,2,110,15,&mThreshold,0,1);
   mAttackSlider = new FloatSlider(this,"attack",5,18,110,15,&mAttackTime,.1f,500);
   mReleaseSlider = new FloatSlider(this,"release",5,34,110,15,&mReleaseTime,.1f,500);
}

void GateEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(GateEffect);

   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   for (int i=0; i<bufferSize; ++i)
   {
      const float decayTime = .01f;
      float scalar = powf( 0.5f, 1.0f/(decayTime * gSampleRate));
      float input = 0;
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         input = MAX(input, fabsf(buffer->GetChannel(ch)[i]));

      if ( input >= mPeak )
      {
         /* When we hit a peak, ride the peak to the top. */
         mPeak = input;
      }
      else
      {
         /* Exponential decay of output when signal is low. */
         mPeak = mPeak * scalar;
         if(mPeak < FLT_EPSILON)
            mPeak = 0.0;
      }

      float sqrtPeak = sqrtf(mPeak);
      if (sqrtPeak >= mThreshold && mEnvelope < 1)
         mEnvelope = MIN(1, mEnvelope+gInvSampleRateMs/mAttackTime );
      if (sqrtPeak < mThreshold && mEnvelope > 0)
         mEnvelope = MAX(0, mEnvelope-gInvSampleRateMs/mReleaseTime );

      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mEnvelope;

      time += gInvSampleRateMs;
   }
}

void GateEffect::DrawModule()
{
   
   mThresholdSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();

   ofPushStyle();
   ofFill();
   ofSetColor(0,255,0,gModuleDrawAlpha*.4f);
   ofRect(5,2,110*sqrtf(mPeak),7);
   ofSetColor(255,0,0,gModuleDrawAlpha*.4f);
   ofRect(5,9,110*mEnvelope,7);
   ofPopStyle();
}

void GateEffect::CheckboxUpdated(Checkbox *checkbox)
{
}

void GateEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void GateEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}


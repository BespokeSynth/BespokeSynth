//
//  KarplusStrongVoice.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "KarplusStrongVoice.h"
#include "EnvOscillator.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Profiler.h"

KarplusStrongVoice::KarplusStrongVoice(IDrawableModule* owner)
: mOscPhase(0)
, mOsc(kOsc_Sin)
, mBuffer(gSampleRate)
, mFilterSample(0)
, mOwner(owner)
{
   mOsc.Start(0, 1);
   ClearVoice();
}

KarplusStrongVoice::~KarplusStrongVoice()
{
}

void KarplusStrongVoice::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("KarplusStrongVoice");

   if (!mActive || mMuteRamp.Value(time) == 0)
      return;
   
   float renderRatio = 1;
   int pitchAdjust = 0;
   if (mVoiceParams->mStretch)
   {
      //pitchAdjust = 12;   //octave up since we're gonna stretch this
      renderRatio = .25f;
   }
   
   int renderSize = bufferSize/renderRatio;
   
   for (int pos=0; pos<renderSize; ++pos)
   {
      if (mOwner)
         mOwner->ComputeSliders(pos);
      
      float pitch = GetPitch(pos) + pitchAdjust;
      
      float filter = ofClamp(ofMap(TheScale->PitchToFreq(pitch),0,880,mVoiceParams->mFilter,0), 0, 1);
      float freq = TheScale->PitchToFreq(pitch);
      
      float oscPhaseInc = 0;
      if (mVoiceParams->mSourceType == kSourceTypeSaw)
      {
         oscPhaseInc = GetPhaseInc(freq);
         mOsc.SetType(kOsc_Saw);
      }
      else
      {
         oscPhaseInc = GetPhaseInc(mVoiceParams->mCarrier + GetModWheel(pos*renderRatio) * 100);
         mOsc.SetType(kOsc_Sin);
      }
      mOscPhase += oscPhaseInc;
      float sample = 0;
      float sinSample = mOsc.Audio(time, mOscPhase);
      float noiseSample = RandomSample();
      float pitchBlend = ofClamp((pitch - 40) / 60.0f,0,1);
      pitchBlend *= pitchBlend;
      if (mVoiceParams->mSourceType == kSourceTypeSin)
         sample = sinSample;
      else if (mVoiceParams->mSourceType == kSourceTypeNoise)
         sample = noiseSample;
      else if (mVoiceParams->mSourceType == kSourceTypeMix)
         sample = noiseSample*pitchBlend + sinSample*(1-pitchBlend);
      else if (mVoiceParams->mSourceType == kSourceTypeSaw)
         sample = mOsc.Audio(time, mOscPhase) / (1+pitchBlend*6);   //quieter at higher pitches
      sample *= mEnv.Value(time) + mVoiceParams->mExcitation;
      //AssertIfDenormal(sample);
      FIX_DENORMAL(sample);

      float samplesAgo = gSampleRate/freq;
      AssertIfDenormal(samplesAgo);
      float feedbackSample = 0;
      if (samplesAgo < mBuffer.Size())
      {
         //interpolated delay
         int pos = int(samplesAgo);
         int posNext = int(samplesAgo)+1;
         if (pos < mBuffer.Size())
         {
            float sample = pos < 0 ? 0 : mBuffer.GetSample(pos);
            float nextSample = posNext >= mBuffer.Size() ? 0 : mBuffer.GetSample(posNext);
            float a = samplesAgo - pos;
            feedbackSample = (1-a)*sample + a*nextSample; //interpolate
            AssertIfDenormal(feedbackSample);
         }
      }
      feedbackSample = (1-filter) * feedbackSample + filter * mFilterSample;
      //AssertIfDenormal(feedbackSample);
      mFilterSample = feedbackSample;
      //sample += mFeedbackRamp.Value(time) * feedbackSample;
      sample += feedbackSample * sqrtf(mVoiceParams->mFeedback) * mMuteRamp.Value(time);
      FIX_DENORMAL(sample);

      mBuffer.Write(sample);

      float output = sample * mVoiceParams->mVol/10.0f * (1 + GetPressure(pos*renderRatio));
      AssertIfDenormal(output);
      
      gWorkBuffer[pos] = output;
      
   }
   
   for (int i=0; i<bufferSize; ++i) //stretch half buffer to fill output
   {
      /*if (i==0)
      {
         out[i] += mLastBufferSample*.5f + mHalfBuffer[0];
      }
      else if (i%2 == 1)*/
      {
         out[i] += gWorkBuffer[int(i/renderRatio)];
      }
      /*else
      {
         out[i] += mHalfBuffer[i/2]*.5f + mHalfBuffer[i/2+1]*.5f;
      }*/
   }
      
   mLastBufferSample = gWorkBuffer[renderSize-1];
}

void KarplusStrongVoice::Start(double time, float target)
{
   mOscPhase = FPI/2;   //magic number that seems to keep things DC centered ok
   mEnv.Clear();
   mEnv.Start(time, target, 3, 0, 1, 3);
   mEnv.SetMaxSustain(10);
   mMuteRamp.SetValue(1);
   mLastBufferSample = 0;
   mActive = true;
}

void KarplusStrongVoice::Stop(double time)
{
   if (mVoiceParams->mMute)
   {
      mMuteRamp.Start(time, 0, time+400);
   }
}

void KarplusStrongVoice::ClearVoice()
{
   mBuffer.ClearBuffer();
   mFilterSample = 0;
   mLastBufferSample = 0;
   mActive = false;
}

void KarplusStrongVoice::SetVoiceParams(IVoiceParams* params)
{
   mVoiceParams = dynamic_cast<KarplusStrongVoiceParams*>(params);
}

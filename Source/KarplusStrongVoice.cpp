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
#include "ChannelBuffer.h"

KarplusStrongVoice::KarplusStrongVoice(IDrawableModule* owner)
: mOscPhase(0)
, mOsc(kOsc_Sin)
, mBuffer(gSampleRate)
, mFilterSample(0)
, mOwner(owner)
{
   mOsc.Start(0, 1);
   mEnv.SetNumStages(2);
   mEnv.GetHasSustainStage() = false;
   mEnv.GetStageData(0).target = 1;
   mEnv.GetStageData(0).time = 3;
   mEnv.GetStageData(1).target = 0;
   mEnv.GetStageData(1).time = 3;
   ClearVoice();
}

KarplusStrongVoice::~KarplusStrongVoice()
{
}

bool KarplusStrongVoice::IsDone(double time)
{
   return !mActive || mMuteRamp.Value(time) == 0;
}

bool KarplusStrongVoice::Process(double time, ChannelBuffer* out)
{
   PROFILER(KarplusStrongVoice);

   if (IsDone(time))
      return false;
   
   int bufferSize = out->BufferSize();
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
         oscPhaseInc = GetPhaseInc(mVoiceParams->mExciterFreq);
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
            float sample = pos < 0 ? 0 : mBuffer.GetSample(pos, 0);
            float nextSample = posNext >= mBuffer.Size() ? 0 : mBuffer.GetSample(posNext, 0);
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

      mBuffer.Write(sample, 0);

      float output = sample * mVoiceParams->mVol/10.0f * (1 + GetPressure(pos*renderRatio));
      AssertIfDenormal(output);
      
      gWorkBuffer[pos] = output;
      
   }
   
   for (int i=0; i<bufferSize; ++i) //stretch half buffer to fill output
   {
      float sample;
      /*if (i==0)
      {
         sample = mLastBufferSample*.5f + mHalfBuffer[0];
      }
      else if (i%2 == 1)*/
      {
         sample = gWorkBuffer[int(i/renderRatio)];
      }
      /*else
      {
         sample = mHalfBuffer[i/2]*.5f + mHalfBuffer[i/2+1]*.5f;
      }*/
      
      if (out->NumActiveChannels() == 1)
      {
         out->GetChannel(0)[i] += sample;
      }
      else
      {
         out->GetChannel(0)[i] += sample * GetLeftPanGain(GetPan());
         out->GetChannel(1)[i] += sample * GetRightPanGain(GetPan());
      }
   }
      
   mLastBufferSample = gWorkBuffer[renderSize-1];
   
   return true;
}

void KarplusStrongVoice::Start(double time, float target)
{
   mOscPhase = FPI/2;   //magic number that seems to keep things DC centered ok
   mEnv.Clear();
   mEnv.GetStageData(0).time = mVoiceParams->mExciterAttack;
   mEnv.GetStageData(1).time = mVoiceParams->mExciterDecay;
   mEnv.Start(time, target);
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

//
//  PolyphonyMgr.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#include "PolyphonyMgr.h"
#include "IMidiVoice.h"
#include "AdditiveVoice.h"
#include "FMVoice.h"
#include "KarplusStrongVoice.h"
#include "SingleOscillatorVoice.h"
#include "SampleVoice.h"
#include "SynthGlobals.h"
#include "Profiler.h"

PolyphonyMgr::PolyphonyMgr(IDrawableModule* owner)
   : mAllowStealing(true)
   , mLastVoice(-1)
   , mFadeOutBufferPos(0)
   , mOwner(owner)
{
   Clear(mFadeOutBuffer, kVoiceFadeSamples);
}

PolyphonyMgr::~PolyphonyMgr()
{
   for (int i=0; i<kNumVoices; ++i)
      delete mVoices[i].mVoice;
}

void PolyphonyMgr::Init(VoiceType type, IVoiceParams* params)
{
   if (type == kVoiceType_Additive)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoices[i].mVoice = new AdditiveVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_FM)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoices[i].mVoice = new FMVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_Karplus)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoices[i].mVoice = new KarplusStrongVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_SingleOscillator)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoices[i].mVoice = new SingleOscillatorVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_Sampler)
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         mVoices[i].mVoice = new SampleVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else
   {
      assert(false);  //unsupported voice type
   }
}

void PolyphonyMgr::Start(double time, int pitch, float amount, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   assert(voiceIdx < kNumVoices);
   
   amount = powf(amount, 1.3f); //increase the importance of velocity
   float pitchScale = 1 / (ofClamp(pitch,1,127)/127.0f); //make lower notes louder
   amount *= pitchScale;
   
   bool preserveVoice = voiceIdx != -1 &&  //we specified a voice
                        mVoices[voiceIdx].mPitch != -1 &&   //there is a note playing from that voice
                        mVoices[voiceIdx].mPitch != pitch;  //and we're not asking to retrigger the same note

   if (voiceIdx == -1) //haven't specified a voice
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         if (mVoices[i].mPitch == pitch)
         {
            voiceIdx = i;   //reuse existing voice
            break;
         }
      }
   }
   
   if (voiceIdx == -1) //need a new voice
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         int check = (i + mLastVoice + 1) % 16;  //try to keep incrementing through list to allow old voices to finish
         if (mVoices[check].mPitch == -1 || mVoices[check].mPitch == pitch)
         {
            voiceIdx = check;
            break;
         }
      }
   }

   if (voiceIdx == -1)   //all used
   {
      if (mAllowStealing)
      {
         double oldest = mVoices[0].mTime;
         int oldestIndex = 0;
         for (int i=1; i<kNumVoices; ++i)
         {
            if (mVoices[i].mTime < oldest)
            {
               oldest = mVoices[i].mTime;
               oldestIndex = i;
            }
         }
         voiceIdx = oldestIndex;
      }
      else
      {
         return;
      }
   }
   
   mVoices[voiceIdx].mPitch = pitch;
   mVoices[voiceIdx].mTime = time;
   IMidiVoice* voice = mVoices[voiceIdx].mVoice;
   assert(voice);
   voice->SetPitch(pitch);
   voice->SetModulators(pitchBend, modWheel, pressure);
   if (!preserveVoice)
   {
      Clear(mFadeOutWriteBuffer, kVoiceFadeSamples);
      voice->Process(time, mFadeOutWriteBuffer, kVoiceFadeSamples);
      for (int i=0; i<kVoiceFadeSamples; ++i)
      {
         float fade = 1 - (float(i) / kVoiceFadeSamples);
         mFadeOutBuffer[(i+mFadeOutBufferPos) % kVoiceFadeSamples] += mFadeOutWriteBuffer[i] * fade;
      }
      voice->ClearVoice();
      voice->Start(time, amount);
   }
   mLastVoice = voiceIdx;
}

void PolyphonyMgr::Stop(double time, int pitch)
{
   for (int i=0; i<kNumVoices; ++i)
   {
      if (mVoices[i].mPitch == pitch)
      {
         mVoices[i].mPitch = -1;
         mVoices[i].mVoice->Stop(time);
      }
   }
}

void PolyphonyMgr::Process(double time, float* out, int bufferSize)
{
   Profiler profiler("PolyphonyMgr");

   for (int i=0; i<kNumVoices; ++i)
   {
      mVoices[i].mVoice->Process(time, out, bufferSize);
   }
   
   for (int i=0; i<bufferSize; ++i)
   {
      int fadeOutIdx = (i+mFadeOutBufferPos) % kVoiceFadeSamples;
      out[i] += mFadeOutBuffer[fadeOutIdx];
      mFadeOutBuffer[fadeOutIdx] = 0;
   }
   mFadeOutBufferPos += bufferSize;
}

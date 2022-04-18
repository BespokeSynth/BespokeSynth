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
//  PolyphonyMgr.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#include "PolyphonyMgr.h"
#include "IMidiVoice.h"
#include "FMVoice.h"
#include "KarplusStrongVoice.h"
#include "SingleOscillatorVoice.h"
#include "SampleVoice.h"
#include "SynthGlobals.h"
#include "Profiler.h"

ChannelBuffer gMidiVoiceWorkChannelBuffer(kWorkBufferSize);

PolyphonyMgr::PolyphonyMgr(IDrawableModule* owner)
: mAllowStealing(true)
, mLastVoice(-1)
, mFadeOutBufferPos(0)
, mOwner(owner)
, mFadeOutBuffer(kVoiceFadeSamples)
, mFadeOutWorkBuffer(kVoiceFadeSamples)
, mVoiceLimit(kNumVoices)
, mOversampling(1)
{
}

PolyphonyMgr::~PolyphonyMgr()
{
   for (int i = 0; i < kNumVoices; ++i)
      delete mVoices[i].mVoice;
}

void PolyphonyMgr::Init(VoiceType type, IVoiceParams* params)
{
   if (type == kVoiceType_FM)
   {
      for (int i = 0; i < kNumVoices; ++i)
      {
         mVoices[i].mVoice = new FMVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_Karplus)
   {
      for (int i = 0; i < kNumVoices; ++i)
      {
         mVoices[i].mVoice = new KarplusStrongVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_SingleOscillator)
   {
      for (int i = 0; i < kNumVoices; ++i)
      {
         mVoices[i].mVoice = new SingleOscillatorVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else if (type == kVoiceType_Sampler)
   {
      for (int i = 0; i < kNumVoices; ++i)
      {
         mVoices[i].mVoice = new SampleVoice(mOwner);
         mVoices[i].mVoice->SetVoiceParams(params);
      }
   }
   else
   {
      assert(false); //unsupported voice type
   }
}

void PolyphonyMgr::Start(double time, int pitch, float amount, int voiceIdx, ModulationParameters modulation)
{
   assert(voiceIdx < kNumVoices);

   bool preserveVoice = voiceIdx != -1 && //we specified a voice
                        mVoices[voiceIdx].mPitch != -1; //there is a note playing from that voice

   /*if (voiceIdx == -1) //haven't specified a voice
   {
      for (int i=0; i<kNumVoices; ++i)
      {
         if (mVoices[i].mPitch == pitch)
         {
            voiceIdx = i;   //reuse existing voice
            preserveVoice = true;
            break;
         }
      }
   }*/

   if (voiceIdx == -1) //need a new voice
   {
      for (int i = 0; i < mVoiceLimit; ++i)
      {
         int check = (i + mLastVoice + 1) % mVoiceLimit; //try to keep incrementing through list to allow old voices to finish
         if (mVoices[check].mPitch == -1)
         {
            voiceIdx = check;
            break;
         }
      }
   }

   if (voiceIdx == -1) //all used
   {
      if (mAllowStealing)
      {
         double oldest = mVoices[0].mTime;
         int oldestIndex = 0;
         for (int i = 1; i < mVoiceLimit; ++i)
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

   IMidiVoice* voice = mVoices[voiceIdx].mVoice;
   assert(voice);
   if (!voice->IsDone(time) && (!preserveVoice || modulation.pan != voice->GetPan()))
   {
      //ofLog() << "fading stolen voice " << voiceIdx << " at " << time;
      mFadeOutWorkBuffer.Clear();
      voice->Process(time, &mFadeOutWorkBuffer, mOversampling);
      for (int i = 0; i < kVoiceFadeSamples; ++i)
      {
         float fade = 1 - (float(i) / kVoiceFadeSamples);
         for (int ch = 0; ch < mFadeOutBuffer.NumActiveChannels(); ++ch)
            mFadeOutBuffer.GetChannel(ch)[(i + mFadeOutBufferPos) % kVoiceFadeSamples] += mFadeOutWorkBuffer.GetChannel(ch)[i] * fade;
      }
   }
   if (!preserveVoice)
      voice->ClearVoice();
   voice->SetPitch(pitch);
   voice->SetModulators(modulation);
   voice->Start(time, amount);
   voice->SetPan(modulation.pan);
   mLastVoice = voiceIdx;

   mVoices[voiceIdx].mPitch = pitch;
   mVoices[voiceIdx].mTime = time;
   mVoices[voiceIdx].mNoteOn = true;
}

void PolyphonyMgr::Stop(double time, int pitch)
{
   for (int i = 0; i < kNumVoices; ++i)
   {
      if (mVoices[i].mPitch == pitch && mVoices[i].mNoteOn)
      {
         mVoices[i].mVoice->Stop(time);
         mVoices[i].mNoteOn = false;
      }
   }
}

void PolyphonyMgr::KillAll()
{
   for (int i = 0; i < kNumVoices; ++i)
   {
      mVoices[i].mVoice->ClearVoice();
      mVoices[i].mNoteOn = false;
   }
}

void PolyphonyMgr::Process(double time, ChannelBuffer* out, int bufferSize)
{
   PROFILER(PolyphonyMgr);

   mFadeOutBuffer.SetNumActiveChannels(out->NumActiveChannels());
   mFadeOutWorkBuffer.SetNumActiveChannels(out->NumActiveChannels());

   for (int i = 0; i < mVoiceLimit; ++i)
   {
      mVoices[i].mVoice->Process(time, out, mOversampling);

      if (mVoices[i].mPitch != -1 && !mVoices[i].mNoteOn && mVoices[i].mVoice->IsDone(time))
         mVoices[i].mPitch = -1;
   }

   for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
   {
      for (int i = 0; i < bufferSize; ++i)
      {
         int fadeOutIdx = (i + mFadeOutBufferPos) % kVoiceFadeSamples;
         out->GetChannel(ch)[i] += mFadeOutBuffer.GetChannel(ch)[fadeOutIdx];
         mFadeOutBuffer.GetChannel(ch)[fadeOutIdx] = 0;
      }
   }

   mFadeOutBufferPos += bufferSize;
}

void PolyphonyMgr::DrawDebug(float x, float y)
{
   ofPushMatrix();
   ofPushStyle();
   ofTranslate(x, y);
   for (int i = 0; i < kNumVoices; ++i)
   {
      if (mVoices[i].mPitch == -1)
         ofSetColor(100, 100, 100);
      else if (mVoices[i].mNoteOn)
         ofSetColor(0, 255, 0);
      else
         ofSetColor(255, 0, 0);
      DrawTextNormal(mVoices[i].mPitch == -1 ? "voice " + ofToString(i) + " unused" : "voice " + ofToString(i) + " used: " + ofToString(mVoices[i].mPitch) + (mVoices[i].mNoteOn ? " (note on)" : " (note off)"), 0, i * 18);
   }
   ofPopStyle();
   ofPopMatrix();
}

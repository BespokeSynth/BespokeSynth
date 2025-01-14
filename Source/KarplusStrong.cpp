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
//  KarplusStrong.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "KarplusStrong.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

KarplusStrong::KarplusStrong()
: IAudioProcessor(gBufferSize)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mPolyMgr.Init(kVoiceType_Karplus, &mVoiceParams);

   AddChild(&mBiquad);
   mBiquad.SetPosition(150, 15);
   mBiquad.SetEnabled(true);
   mBiquad.SetFilterType(kFilterType_Lowpass);
   mBiquad.SetFilterParams(3000, sqrt(2) / 2);
   mBiquad.SetName("biquad");

   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mDCRemover[i].SetFilterParams(10, sqrt(2) / 2);
      mDCRemover[i].SetFilterType(kFilterType_Highpass);
      mDCRemover[i].UpdateFilterCoeff();
   }
}

void KarplusStrong::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this, "vol", 3, 2, 80, 15, &mVolume, 0, 2);
   mInvertCheckbox = new Checkbox(this, "invert", mVolSlider, kAnchor_Right, &mVoiceParams.mInvert);
   mFilterSlider = new FloatSlider(this, "filter", mVolSlider, kAnchor_Below, 140, 15, &mVoiceParams.mFilter, 0, 5);
   mFeedbackSlider = new FloatSlider(this, "feedback", mFilterSlider, kAnchor_Below, 140, 15, &mVoiceParams.mFeedback, .5f, .9999f, 4);
   mSourceDropdown = new DropdownList(this, "source type", mFeedbackSlider, kAnchor_Below, (int*)&mVoiceParams.mSourceType, 52);
   mExciterFreqSlider = new FloatSlider(this, "x freq", mSourceDropdown, kAnchor_Right, 85, 15, &mVoiceParams.mExciterFreq, 10, 1000);
   mExciterAttackSlider = new FloatSlider(this, "x att", mSourceDropdown, kAnchor_Below, 69, 15, &mVoiceParams.mExciterAttack, 0.01f, 40);
   mExciterDecaySlider = new FloatSlider(this, "x dec", mExciterAttackSlider, kAnchor_Right, 68, 15, &mVoiceParams.mExciterDecay, 0.01f, 40);
   mVelToVolumeSlider = new FloatSlider(this, "vel2vol", mExciterAttackSlider, kAnchor_Below, 140, 15, &mVoiceParams.mVelToVolume, 0, 1);
   mVelToEnvelopeSlider = new FloatSlider(this, "vel2env", mVelToVolumeSlider, kAnchor_Below, 140, 15, &mVoiceParams.mVelToEnvelope, -1, 1);
   mPitchToneSlider = new FloatSlider(this, "pitchtone", mVelToVolumeSlider, kAnchor_Right, 125, 15, &mVoiceParams.mPitchTone, -2, 2);
   mLiteCPUModeCheckbox = new Checkbox(this, "lite cpu", mPitchToneSlider, kAnchor_Below, &mVoiceParams.mLiteCPUMode);
   //mStretchCheckbox = new Checkbox(this,"stretch",mVolSlider,kAnchor_Right,&mVoiceParams.mStretch);

   mSourceDropdown->AddLabel("sin", kSourceTypeSin);
   mSourceDropdown->AddLabel("white", kSourceTypeNoise);
   mSourceDropdown->AddLabel("mix", kSourceTypeMix);
   mSourceDropdown->AddLabel("saw", kSourceTypeSaw);
   mSourceDropdown->AddLabel("input", kSourceTypeInput);
   mSourceDropdown->AddLabel("input*", kSourceTypeInputNoEnvelope);

   mFilterSlider->SetMode(FloatSlider::kSquare);
   mExciterFreqSlider->SetMode(FloatSlider::kSquare);
   mExciterAttackSlider->SetMode(FloatSlider::kSquare);
   mExciterDecaySlider->SetMode(FloatSlider::kSquare);

   mBiquad.CreateUIControls();
}

KarplusStrong::~KarplusStrong()
{
}

void KarplusStrong::Process(double time)
{
   PROFILER(KarplusStrong);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   SyncBuffers(mWriteBuffer.NumActiveChannels());

   mNoteInputBuffer.Process(time);

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);

   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      Mult(mWriteBuffer.GetChannel(ch), mVolume, bufferSize);
      if (!mVoiceParams.mInvert) //unnecessary if inversion is eliminating dc offset
         mDCRemover[ch].Filter(mWriteBuffer.GetChannel(ch), bufferSize);
   }

   mBiquad.ProcessAudio(time, &mWriteBuffer);

   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }

   GetBuffer()->Reset();
}

void KarplusStrong::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity > 0)
      mPolyMgr.Start(note.time, note.pitch, note.velocity / 127.0f, note.voiceIdx, note.modulation);
   else
      mPolyMgr.Stop(note.time, note.pitch, note.voiceIdx);
}

void KarplusStrong::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void KarplusStrong::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mFilterSlider->Draw();
   mFeedbackSlider->Draw();
   mVolSlider->Draw();
   mSourceDropdown->Draw();
   mInvertCheckbox->Draw();
   mPitchToneSlider->Draw();
   mVelToVolumeSlider->Draw();
   mVelToEnvelopeSlider->Draw();
   mLiteCPUModeCheckbox->Draw();

   mExciterFreqSlider->SetShowing(mVoiceParams.mSourceType == kSourceTypeSin || mVoiceParams.mSourceType == kSourceTypeSaw || mVoiceParams.mSourceType == kSourceTypeMix);
   mExciterAttackSlider->SetShowing(mVoiceParams.mSourceType != kSourceTypeInputNoEnvelope);
   mExciterDecaySlider->SetShowing(mVoiceParams.mSourceType != kSourceTypeInputNoEnvelope);

   //mStretchCheckbox->Draw();
   mExciterFreqSlider->Draw();
   mExciterAttackSlider->Draw();
   mExciterDecaySlider->Draw();

   mBiquad.Draw();
}

void KarplusStrong::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(250, 0);
   }
}

void KarplusStrong::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void KarplusStrong::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void KarplusStrong::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mPolyMgr.KillAll();
      for (int ch = 0; ch < ChannelBuffer::kMaxNumChannels; ++ch)
         mDCRemover[ch].Clear();
      mBiquad.Clear();
   }
}

void KarplusStrong::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);
   EnumMap oversamplingMap;
   oversamplingMap["1"] = 1;
   oversamplingMap["2"] = 2;
   oversamplingMap["4"] = 4;
   oversamplingMap["8"] = 8;
   mModuleSaveData.LoadEnum<int>("oversampling", moduleInfo, 1, nullptr, &oversamplingMap);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);

   SetUpFromSaveData();
}

void KarplusStrong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);
   else
      mPolyMgr.SetVoiceLimit(kNumVoices);

   bool mono = mModuleSaveData.GetBool("mono");
   mWriteBuffer.SetNumActiveChannels(mono ? 1 : 2);

   int oversampling = mModuleSaveData.GetEnum<int>("oversampling");
   mPolyMgr.SetOversampling(oversampling);
}

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
//  PadSynth.cpp
//  modularSynth
//
//  Created by Andrius Merkys on 4/8/25.
//
//

#include "PadSynth.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

namespace
{
   const float kColumnWidth = 90;
   const float kGap = 5;
}

PadSynth::PadSynth()
: mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mPolyMgr.Init(kVoiceType_PadSynth, &mVoiceParams);

   AddChild(&mBiquad);
   mBiquad.SetPosition(3 + (kGap + kColumnWidth) * 2, 15);
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

void PadSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float width, height;

   UIBLOCK(3, 3, kColumnWidth);
   FLOATSLIDER(mBandwidthSlider, "bandwidth", &mVoiceParams.mBandwidth, 0.01f, 100);
   INTSLIDER(mHarmonicsSlider, "harmonics", &mVoiceParams.mHarmonics, 1, 128);
   FLOATSLIDER(mDetuneSlider, "detune", &mVoiceParams.mDetune, 0.0, 1.0);
   FLOATSLIDER(mBandwidthScaleSlider, "scale", &mVoiceParams.mBandwidthScale, 0.01f, 2);
   ENDUIBLOCK(width, height);

   UIBLOCK(3 + kGap + kColumnWidth, 3, kColumnWidth);
   UICONTROL_CUSTOM(mADSRDisplay, new ADSRDisplay(UICONTROL_BASICS("env"), kColumnWidth, 36, &mVoiceParams.mAdsr));
   FLOATSLIDER(mVolSlider, "vol", &mVolume, 0, 2);
   FLOATSLIDER(mChannelOffsetSlider, "offset", &mVoiceParams.mChannelOffset, 0, 1);
   //CHECKBOX(mLiteCPUModeCheckbox, "lite cpu", &mVoiceParams.mLiteCPUMode);
   ENDUIBLOCK(width, height);

   mBiquad.CreateUIControls();
}

PadSynth::~PadSynth()
{
}

void PadSynth::Process(double time)
{
   PROFILER(PadSynth);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   mNoteInputBuffer.Process(time);

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      Mult(mWriteBuffer.GetChannel(ch), mVolume, bufferSize);
	  mDCRemover[ch].Filter(mWriteBuffer.GetChannel(ch), bufferSize);
   }

   mBiquad.ProcessAudio(time, &mWriteBuffer);

   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void PadSynth::PlayNote(NoteMessage note)
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

void PadSynth::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void PadSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mBandwidthSlider->Draw();
   mHarmonicsSlider->Draw();
   mBandwidthScaleSlider->Draw();
   mDetuneSlider->Draw();
   mADSRDisplay->Draw();
   mVolSlider->Draw();
   mChannelOffsetSlider->Draw();

   mBiquad.Draw();
}

void PadSynth::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(250, 0);
   }
}

void PadSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void PadSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void PadSynth::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void PadSynth::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mPolyMgr.KillAll();
      for (int ch = 0; ch < ChannelBuffer::kMaxNumChannels; ++ch)
         mDCRemover[ch].Clear();
      mBiquad.Clear();
   }
}

void PadSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);
   EnumMap oversamplingMap;
   oversamplingMap["1"] = 1;
   oversamplingMap["2"] = 2;
   oversamplingMap["4"] = 4;
   oversamplingMap["8"] = 8;
   mModuleSaveData.LoadEnum<int>("oversampling", moduleInfo, 1, nullptr, &oversamplingMap);
   mModuleSaveData.LoadInt("undersampling", moduleInfo, 0, 0, 16);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);

   SetUpFromSaveData();
}

void PadSynth::SetUpFromSaveData()
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

   int undersampling = mModuleSaveData.GetInt("undersampling");
   mVoiceParams.mUndersample = undersampling;
}

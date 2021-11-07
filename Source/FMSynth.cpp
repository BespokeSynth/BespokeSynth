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
//  FMSynth.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#include "FMSynth.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

FMSynth::FMSynth()
: mHarmRatioBase(1)
, mHarmRatioBaseDropdown(nullptr)
, mHarmRatioTweak(1)
, mHarmRatioBase2(1)
, mHarmRatioBaseDropdown2(nullptr)
, mHarmRatioTweak2(1)
, mHarmSlider(nullptr)
, mModSlider(nullptr)
, mHarmSlider2(nullptr)
, mModSlider2(nullptr)
, mVolSlider(nullptr)
, mAdsrDisplayVol(nullptr)
, mAdsrDisplayHarm(nullptr)
, mAdsrDisplayMod(nullptr)
, mAdsrDisplayHarm2(nullptr)
, mAdsrDisplayMod2(nullptr)
, mPhaseOffsetSlider0(nullptr)
, mPhaseOffsetSlider1(nullptr)
, mPhaseOffsetSlider2(nullptr)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mVoiceParams.mOscADSRParams.GetA() = 10;
   mVoiceParams.mOscADSRParams.GetD() = 0;
   mVoiceParams.mOscADSRParams.GetS() = 1;
   mVoiceParams.mOscADSRParams.GetR() = 10;
   mVoiceParams.mHarmRatioADSRParams.GetA() = 1;
   mVoiceParams.mHarmRatioADSRParams.GetD() = 0;
   mVoiceParams.mHarmRatioADSRParams.GetS() = 1;
   mVoiceParams.mHarmRatioADSRParams.GetR() = 1;
   mVoiceParams.mModIdxADSRParams.GetA() = 1;
   mVoiceParams.mModIdxADSRParams.GetD() = 0;
   mVoiceParams.mModIdxADSRParams.GetS() = 1;
   mVoiceParams.mModIdxADSRParams.GetR() = 1;
   mVoiceParams.mPhaseOffset0 = 0;
   mVoiceParams.mHarmRatio = 1;
   mVoiceParams.mModIdx = 0;
   mVoiceParams.mPhaseOffset1 = 0;
   mVoiceParams.mHarmRatioADSRParams2.GetA() = 1;
   mVoiceParams.mHarmRatioADSRParams2.GetD() = 0;
   mVoiceParams.mHarmRatioADSRParams2.GetS() = 1;
   mVoiceParams.mHarmRatioADSRParams2.GetR() = 1;
   mVoiceParams.mModIdxADSRParams2.GetA() = 1;
   mVoiceParams.mModIdxADSRParams2.GetD() = 0;
   mVoiceParams.mModIdxADSRParams2.GetS() = 1;
   mVoiceParams.mModIdxADSRParams2.GetR() = 1;
   mVoiceParams.mHarmRatio2 = 1;
   mVoiceParams.mModIdx2 = 0;
   mVoiceParams.mPhaseOffset2 = 0;
   mVoiceParams.mVol = 1.f;

   mPolyMgr.Init(kVoiceType_FM, &mVoiceParams);
}

void FMSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAdsrDisplayVol = new ADSRDisplay(this,"adsrosc",4,4,80,40,&mVoiceParams.mOscADSRParams);
   mAdsrDisplayHarm = new ADSRDisplay(this,"adsrharm",4,50,80,40,&mVoiceParams.mHarmRatioADSRParams);
   mAdsrDisplayMod = new ADSRDisplay(this,"adsrmod",94,50,80,40,&mVoiceParams.mModIdxADSRParams);
   mAdsrDisplayHarm2 = new ADSRDisplay(this,"adsrharm2",4,127,80,40,&mVoiceParams.mHarmRatioADSRParams2);
   mAdsrDisplayMod2 = new ADSRDisplay(this,"adsrmod2",94,127,80,40,&mVoiceParams.mModIdxADSRParams2);
   mHarmRatioBaseDropdown = new DropdownList(this,"harmratio",mAdsrDisplayHarm,kAnchor_Below,&mHarmRatioBase);
   mHarmRatioBaseDropdown2 = new DropdownList(this,"harmratio2",mAdsrDisplayHarm2,kAnchor_Below,&mHarmRatioBase2);
   mHarmSlider = new FloatSlider(this,"tweak",mHarmRatioBaseDropdown,kAnchor_Below,80,15,&mHarmRatioTweak,.5f,2,3);
   mModSlider = new FloatSlider(this,"mod",mAdsrDisplayMod,kAnchor_Below,80,15,&mVoiceParams.mModIdx,0,20);
   mHarmSlider2 = new FloatSlider(this,"tweak2",mHarmRatioBaseDropdown2,kAnchor_Below,80,15,&mHarmRatioTweak2,.5f,2,3);
   mModSlider2 = new FloatSlider(this,"mod2",mAdsrDisplayMod2,kAnchor_Below,80,15,&mVoiceParams.mModIdx2,0,20);
   mVolSlider = new FloatSlider(this,"vol",94,4,80,15,&mVoiceParams.mVol,0,2);
   mPhaseOffsetSlider0 = new FloatSlider(this,"phase0",mVolSlider,kAnchor_Below,80,15,&mVoiceParams.mPhaseOffset0,0,FTWO_PI);
   mPhaseOffsetSlider1 = new FloatSlider(this,"phase1",mModSlider,kAnchor_Below,80,15,&mVoiceParams.mPhaseOffset1,0,FTWO_PI);
   mPhaseOffsetSlider2 = new FloatSlider(this,"phase2",mModSlider2,kAnchor_Below,80,15,&mVoiceParams.mPhaseOffset2,0,FTWO_PI);
   
   mHarmRatioBaseDropdown->AddLabel(".125", -8);
   mHarmRatioBaseDropdown->AddLabel(".2", -5);
   mHarmRatioBaseDropdown->AddLabel(".25", -4);
   mHarmRatioBaseDropdown->AddLabel(".333", -3);
   mHarmRatioBaseDropdown->AddLabel(".5", -2);
   mHarmRatioBaseDropdown->AddLabel("1", 1);
   mHarmRatioBaseDropdown->AddLabel("2", 2);
   mHarmRatioBaseDropdown->AddLabel("3", 3);
   mHarmRatioBaseDropdown->AddLabel("4", 4);
   mHarmRatioBaseDropdown->AddLabel("8", 8);
   mHarmRatioBaseDropdown->AddLabel("16", 16);
   
   mHarmRatioBaseDropdown2->AddLabel(".125", -8);
   mHarmRatioBaseDropdown2->AddLabel(".2", -5);
   mHarmRatioBaseDropdown2->AddLabel(".25", -4);
   mHarmRatioBaseDropdown2->AddLabel(".333", -3);
   mHarmRatioBaseDropdown2->AddLabel(".5", -2);
   mHarmRatioBaseDropdown2->AddLabel("1", 1);
   mHarmRatioBaseDropdown2->AddLabel("2", 2);
   mHarmRatioBaseDropdown2->AddLabel("3", 3);
   mHarmRatioBaseDropdown2->AddLabel("4", 4);
   mHarmRatioBaseDropdown2->AddLabel("8", 8);
   mHarmRatioBaseDropdown2->AddLabel("16", 16);
   
   mModSlider->SetMode(FloatSlider::kSquare);
   mModSlider2->SetMode(FloatSlider::kSquare);
}

FMSynth::~FMSynth()
{
}

void FMSynth::Process(double time)
{
   PROFILER(FMSynth);

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
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void FMSynth::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
      mVoiceParams.mOscADSRParams.Start(time,1);   //for visualization
   }
   else
   {
      mPolyMgr.Stop(time, pitch);
      mVoiceParams.mOscADSRParams.Stop(time);   //for visualization
   }

   if (mDrawDebug)
   {
      std::vector<std::string> lines = ofSplitString(mDebugLines, "\n");
      mDebugLines = "";
      const int kNumDisplayLines = 10;
      for (int i = 0; i < kNumDisplayLines - 1; ++i)
      {
         int lineIndex = (int)lines.size() - (kNumDisplayLines - 1) + i;
         if (lineIndex >= 0)
            mDebugLines += lines[lineIndex] + "\n";
      }
      std::string debugLine = "PlayNote(" + ofToString(time / 1000) + ", " + ofToString(pitch) + ", " + ofToString(velocity) + ", " + ofToString(voiceIdx) + ")";
      mDebugLines += debugLine;
      ofLog() << debugLine;
   }
}

void FMSynth::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void FMSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAdsrDisplayVol->Draw();
   mAdsrDisplayHarm->Draw();
   mAdsrDisplayMod->Draw();
   mHarmSlider->Draw();
   mModSlider->Draw();
   mVolSlider->Draw();
   mPhaseOffsetSlider0->Draw();
   mHarmRatioBaseDropdown->Draw();
   mPhaseOffsetSlider1->Draw();
   mAdsrDisplayHarm2->Draw();
   mAdsrDisplayMod2->Draw();
   mHarmSlider2->Draw();
   mModSlider2->Draw();
   mHarmRatioBaseDropdown2->Draw();
   mPhaseOffsetSlider2->Draw();

   DrawTextNormal("env",mAdsrDisplayVol->GetPosition(true).x, mAdsrDisplayVol->GetPosition(true).y+10);
   DrawTextNormal("harm",mAdsrDisplayHarm->GetPosition(true).x, mAdsrDisplayHarm->GetPosition(true).y+10);
   DrawTextNormal("mod",mAdsrDisplayMod->GetPosition(true).x, mAdsrDisplayMod->GetPosition(true).y+10);
   DrawTextNormal("harm2",mAdsrDisplayHarm2->GetPosition(true).x, mAdsrDisplayHarm2->GetPosition(true).y+10);
   DrawTextNormal("mod2",mAdsrDisplayMod2->GetPosition(true).x, mAdsrDisplayMod2->GetPosition(true).y+10);
}

void FMSynth::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      float width, height;
      GetModuleDimensions(width, height);
      mPolyMgr.DrawDebug(width + 3, 0);
      DrawTextNormal(mDebugLines, 0, height + 15);
   }
}

void FMSynth::UpdateHarmonicRatio()
{
   if (mHarmRatioBase < 0)
      mVoiceParams.mHarmRatio = 1.0f/(-mHarmRatioBase);
   else
      mVoiceParams.mHarmRatio = mHarmRatioBase;
   mVoiceParams.mHarmRatio *= mHarmRatioTweak;
   
   if (mHarmRatioBase2 < 0)
      mVoiceParams.mHarmRatio2 = 1.0f/(-mHarmRatioBase2);
   else
      mVoiceParams.mHarmRatio2 = mHarmRatioBase2;
   mVoiceParams.mHarmRatio2 *= mHarmRatioTweak2;
}

void FMSynth::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mHarmRatioBaseDropdown || list == mHarmRatioBaseDropdown2)
      UpdateHarmonicRatio();
}

void FMSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mHarmSlider || slider == mHarmSlider2)
      UpdateHarmonicRatio();
}

void FMSynth::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mPolyMgr.KillAll();
}

void FMSynth::LoadLayout(const ofxJSONElement& moduleInfo)
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

void FMSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);

   bool mono = mModuleSaveData.GetBool("mono");
   mWriteBuffer.SetNumActiveChannels(mono ? 1 : 2);

   int oversampling = mModuleSaveData.GetEnum<int>("oversampling");
   mPolyMgr.SetOversampling(oversampling);
}



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
   
   mWriteBuffer.SetNumActiveChannels(2);
}

FMSynth::~FMSynth()
{
}

void FMSynth::Process(double time)
{
   PROFILER(FMSynth);

   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   mNoteInputBuffer.Process(time);
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   
   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(GetTarget()->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void FMSynth::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!NoteInputBuffer::IsTimeWithinFrame(time))
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

void FMSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FMSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}



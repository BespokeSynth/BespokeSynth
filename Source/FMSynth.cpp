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
, mHarmRatioTweak(0)
, mHarmSlider(nullptr)
, mModSlider(nullptr)
, mVolSlider(nullptr)
, mAdsrDisplayOsc(nullptr)
, mAdsrDisplayHarm(nullptr)
, mAdsrDisplayMod(nullptr)
, mPolyMgr(this)
{
   mVoiceParams.mOscADSRParams.mA = 10;
   mVoiceParams.mOscADSRParams.mD = 0;
   mVoiceParams.mOscADSRParams.mS = 1;
   mVoiceParams.mOscADSRParams.mR = 10;
   mVoiceParams.mHarmRatioADSRParams.mA = 1;
   mVoiceParams.mHarmRatioADSRParams.mD = 0;
   mVoiceParams.mHarmRatioADSRParams.mS = 1;
   mVoiceParams.mHarmRatioADSRParams.mR = 1;
   mVoiceParams.mModIdxADSRParams.mA = 1;
   mVoiceParams.mModIdxADSRParams.mD = 0;
   mVoiceParams.mModIdxADSRParams.mS = 1;
   mVoiceParams.mModIdxADSRParams.mR = 1;
   mVoiceParams.mHarmRatio = 1;
   mVoiceParams.mModIdx = 0;
   mVoiceParams.mVol = 1.f;

   mPolyMgr.Init(kVoiceType_FM, &mVoiceParams);

   mWriteBuffer = new float[gBufferSize];
}

void FMSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mHarmRatioBaseDropdown = new DropdownList(this,"harmratio",66,46,&mHarmRatioBase);
   mHarmSlider = new FloatSlider(this,"tweak",107,46,77,15,&mHarmRatioTweak,-0.01f,0.01f,3);
   mModSlider = new FloatSlider(this,"mod",186,46,77,15,&mVoiceParams.mModIdx,0,20);
   mVolSlider = new FloatSlider(this,"vol",4,46,60,15,&mVoiceParams.mVol,0,2);
   mAdsrDisplayOsc = new ADSRDisplay(this,"adsrosc",4,4,80,40,&mVoiceParams.mOscADSRParams);
   mAdsrDisplayHarm = new ADSRDisplay(this,"adsrharm",94,4,80,40,&mVoiceParams.mHarmRatioADSRParams);
   mAdsrDisplayMod = new ADSRDisplay(this,"adsrmod",184,4,80,40,&mVoiceParams.mModIdxADSRParams);
   
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
   
   mModSlider->SetMode(FloatSlider::kSquare);
}

FMSynth::~FMSynth()
{
   delete[] mWriteBuffer;
}

void FMSynth::Process(double time)
{
   Profiler profiler("FMSynth");

   if (!mEnabled || GetTarget() == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   Clear(mWriteBuffer, gBufferSize);
   mPolyMgr.Process(time, mWriteBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);

   Add(out, mWriteBuffer, gBufferSize);
}

void FMSynth::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, pitchBend, modWheel, pressure);
   else
      mPolyMgr.Stop(time, pitch);
}

void FMSynth::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void FMSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAdsrDisplayOsc->Draw();
   mAdsrDisplayHarm->Draw();
   mAdsrDisplayMod->Draw();
   mHarmSlider->Draw();
   mModSlider->Draw();
   mVolSlider->Draw();
   mHarmRatioBaseDropdown->Draw();

   DrawText("adsr",8,14);
   DrawText("harm",98,14);
   DrawText("mod",188,14);
}

void FMSynth::UpdateHarmonicRatio()
{
   if (mHarmRatioBase < 0)
      mVoiceParams.mHarmRatio = 1.0f/(-mHarmRatioBase);
   else
      mVoiceParams.mHarmRatio = mHarmRatioBase;
   mVoiceParams.mHarmRatio += mHarmRatioTweak;
   
}

void FMSynth::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mHarmRatioBaseDropdown)
      UpdateHarmonicRatio();
}

void FMSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mHarmSlider)
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



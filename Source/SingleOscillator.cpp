//
//  SingleOscillator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#include "SingleOscillator.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Profiler.h"

SingleOscillator::SingleOscillator()
: mVolSlider(NULL)
, mOscSelector(NULL)
, mPulseWidthSlider(NULL)
, mMult(1)
, mMultSelector(NULL)
, mADSRDisplay(NULL)
, mSyncCheckbox(NULL)
, mSyncFreqSlider(NULL)
, mDetuneSlider(NULL)
, mADSRModeSelector(NULL)
, mADSRMode(0)
, mShuffleSlider(NULL)
, mPolyMgr(this)
, mLengthMultiplier(1)
, mLengthMultiplierSlider(nullptr)
{
   mVoiceParams.mAdsr.Set(10,0,1,10);
   mVoiceParams.mVol = .05f;
   mVoiceParams.mPulseWidth = .5f;
   mVoiceParams.mSync = false;
   mVoiceParams.mSyncFreq = 200;
   mVoiceParams.mMult = 1;
   mVoiceParams.mOscType = kOsc_Square;
   mVoiceParams.mDetune = 1;
   mVoiceParams.mFilterAdsr.Set(1,0,1,30);
   mVoiceParams.mFilterCutoff = SINGLEOSCILLATOR_NO_CUTOFF;
   mVoiceParams.mPressureEnvelope = false;
   mVoiceParams.mShuffle = 0;
   
   mPolyMgr.Init(kVoiceType_SingleOscillator, &mVoiceParams);
   
   mWriteBuffer = new float[gBufferSize];
}

void SingleOscillator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",5,73,80,15,&mVoiceParams.mVol,0,1);
   mLengthMultiplierSlider = new FloatSlider(this,"len",5,56,80,15,&mLengthMultiplier,.01f,10);
   mOscSelector = new DropdownList(this,"osc",70,1,(int*)(&mVoiceParams.mOscType));
   mPulseWidthSlider = new FloatSlider(this,"pw",117,1,60,15,&mVoiceParams.mPulseWidth,0.01f,.99f,2);
   mMultSelector = new DropdownList(this,"mult",48,19,&mMult);
   mADSRDisplay = new ADSRDisplay(this,"adsr",5,19,80,36,&mVoiceParams.mAdsr);
   mSyncCheckbox = new Checkbox(this,"sync",95,38,&mVoiceParams.mSync);
   mSyncFreqSlider = new FloatSlider(this,"syncf",135,38,40,15,&mVoiceParams.mSyncFreq,10,999.9f);
   mDetuneSlider = new FloatSlider(this,"detune",95,72,80,15,&mVoiceParams.mDetune,.98f,1.02f,3);
   mShuffleSlider = new FloatSlider(this,"shuffle",95,55,80,15,&mVoiceParams.mShuffle, 0, 1);
   mFilterADSRDisplay = new ADSRDisplay(this,"adsrfilter",5,19,80,36,&mVoiceParams.mFilterAdsr);
   mADSRModeSelector = new RadioButton(this,"adsrmode",5,1,&mADSRMode,kRadioHorizontal);
   mFilterCutoffSlider = new FloatSlider(this,"cutoff",95,21,80,15,&mVoiceParams.mFilterCutoff,0,SINGLEOSCILLATOR_NO_CUTOFF);

   mSyncFreqSlider->SetLabel("");
   
   mFilterCutoffSlider->SetMaxValueDisplay("none");
   
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("noise",kOsc_Random);
   
   mMultSelector->AddLabel("8", 8);
   mMultSelector->AddLabel("7", 7);
   mMultSelector->AddLabel("6", 6);
   mMultSelector->AddLabel("5", 5);
   mMultSelector->AddLabel("4", 4);
   mMultSelector->AddLabel("3", 3);
   mMultSelector->AddLabel("2", 2);
   mMultSelector->AddLabel("1", 1);
   mMultSelector->AddLabel("1/2", -2);
   mMultSelector->AddLabel("1/3", -3);
   mMultSelector->AddLabel("1/4", -4);
   mMultSelector->AddLabel("1/5", -5);
   mMultSelector->AddLabel("1/6", -6);
   mMultSelector->AddLabel("1/7", -7);
   mMultSelector->AddLabel("1/8", -8);
   
   mADSRDisplay->SetVol(mVoiceParams.mVol);
   
   mADSRModeSelector->AddLabel("vol",0);
   mADSRModeSelector->AddLabel("filter",1);
   
   UpdateADSRDisplays();
}

SingleOscillator::~SingleOscillator()
{
   delete[] mWriteBuffer;
}

void SingleOscillator::Process(double time)
{
   Profiler profiler("SingleOscillator");

   if (!mEnabled || GetTarget() == NULL)
      return;

   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   mPolyMgr.Process(time, mWriteBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SingleOscillator::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, pitchBend, modWheel, pressure);
   else
      mPolyMgr.Stop(time, pitch);
}

void SingleOscillator::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void SingleOscillator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mLengthMultiplierSlider->Draw();
   mPulseWidthSlider->Draw();
   mSyncCheckbox->Draw();
   mSyncFreqSlider->Draw();
   mOscSelector->Draw();
   mDetuneSlider->Draw();
   mShuffleSlider->Draw();
   mFilterADSRDisplay->Draw();
   mFilterCutoffSlider->Draw();
   if (mADSRMode == 0)  //draw order
   {
      mADSRModeSelector->Draw();
      mADSRDisplay->Draw();
   }
   else
   {
      mADSRDisplay->Draw();
      mADSRModeSelector->Draw();
   }
   mMultSelector->Draw();
}

void SingleOscillator::GetModuleDimensions(int& width, int& height)
{
   width = 180;
   height = 90;
}

void SingleOscillator::UpdateADSRDisplays()
{
   mADSRDisplay->SetActive(mADSRMode == 0);
   mFilterADSRDisplay->SetActive(mADSRMode == 1);
}

void SingleOscillator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("vol", moduleInfo, 1, mVolSlider);
   mModuleSaveData.LoadEnum<OscillatorType>("osc", moduleInfo, kOsc_Sin, mOscSelector);
   mModuleSaveData.LoadFloat("detune", moduleInfo, 1, mDetuneSlider);
   mModuleSaveData.LoadBool("pressure_envelope", moduleInfo);

   SetUpFromSaveData();
}

void SingleOscillator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetVol(mModuleSaveData.GetFloat("vol"));
   SetType(mModuleSaveData.GetEnum<OscillatorType>("osc"));
   SetDetune(mModuleSaveData.GetFloat("detune"));
   mVoiceParams.mPressureEnvelope = mModuleSaveData.GetBool("pressure_envelope");
}


void SingleOscillator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mMultSelector)
      mVoiceParams.mMult = mMult >= 0 ? mMult : -1.0f/mMult;
}

void SingleOscillator::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mADSRModeSelector)
   {
      UpdateADSRDisplays();
   }
}

void SingleOscillator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mVolSlider)
      mADSRDisplay->SetVol(mVoiceParams.mVol);
   if (slider == mLengthMultiplierSlider)
   {
      mADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
      mFilterADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
   }
}

void SingleOscillator::IntSliderUpdated(IntSlider* slider, int oldVal)
{

}

void SingleOscillator::CheckboxUpdated(Checkbox* checkbox)
{
}


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
: mVolSlider(nullptr)
, mPhaseOffsetSlider(nullptr)
, mOscSelector(nullptr)
, mPulseWidthSlider(nullptr)
, mMult(1)
, mMultSelector(nullptr)
, mADSRDisplay(nullptr)
, mSyncCheckbox(nullptr)
, mSyncFreqSlider(nullptr)
, mDetuneSlider(nullptr)
, mADSRModeSelector(nullptr)
, mADSRMode(0)
, mShuffleSlider(nullptr)
, mPolyMgr(this)
, mLengthMultiplier(1)
, mLengthMultiplierSlider(nullptr)
, mDrawOsc(kOsc_Sin)
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
   mVoiceParams.mPhaseOffset = 0;
   
   mPolyMgr.Init(kVoiceType_SingleOscillator, &mVoiceParams);
   
   mWriteBuffer = new float[gBufferSize];
}

void SingleOscillator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mOscSelector = new DropdownList(this,"osc",5,1,(int*)(&mVoiceParams.mOscType));
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("noise",kOsc_Random);
   mMultSelector = new DropdownList(this,"mult",mOscSelector,kAnchor_Right,&mMult);
   mPulseWidthSlider = new FloatSlider(this,"pw",5,55,80,15,&mVoiceParams.mPulseWidth,0.01f,.99f,2);
   mShuffleSlider = new FloatSlider(this,"shuffle",mPulseWidthSlider,kAnchor_Below,80,15,&mVoiceParams.mShuffle, 0, 1);
   mDetuneSlider = new FloatSlider(this,"detune",mShuffleSlider,kAnchor_Below,80,15,&mVoiceParams.mDetune,.98f,1.02f,3);
   
   mADSRModeSelector = new RadioButton(this,"envmode",95,1,&mADSRMode,kRadioHorizontal);
   mADSRDisplay = new ADSRDisplay(this,"env",95,18,80,36,&mVoiceParams.mAdsr);
   mFilterADSRDisplay = new ADSRDisplay(this,"envfilter",95,18,80,36,&mVoiceParams.mFilterAdsr);
   mLengthMultiplierSlider = new FloatSlider(this,"len",mADSRDisplay,kAnchor_Below,80,15,&mLengthMultiplier,.01f,10);
   mVolSlider = new FloatSlider(this,"vol",mLengthMultiplierSlider,kAnchor_Below,80,15,&mVoiceParams.mVol,0,1);
   mPhaseOffsetSlider = new FloatSlider(this,"phase",mDetuneSlider,kAnchor_Below,80,15,&mVoiceParams.mPhaseOffset,0,TWO_PI);
   mFilterCutoffSlider = new FloatSlider(this,"cutoff",mVolSlider,kAnchor_Below,80,15,&mVoiceParams.mFilterCutoff,0,SINGLEOSCILLATOR_NO_CUTOFF);
   mSyncCheckbox = new Checkbox(this,"sync",mFilterCutoffSlider,kAnchor_Below,&mVoiceParams.mSync);
   mSyncFreqSlider = new FloatSlider(this,"syncf",mSyncCheckbox,kAnchor_Right,40,15,&mVoiceParams.mSyncFreq,10,999.9f);

   mSyncFreqSlider->SetLabel("");
   
   mFilterCutoffSlider->SetMaxValueDisplay("none");
   
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

   if (!mEnabled || GetTarget() == nullptr)
      return;

   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   mPolyMgr.Process(time, mWriteBuffer, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SingleOscillator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
      mVoiceParams.mAdsr.Start(time,1);         //for visualization
      mVoiceParams.mFilterAdsr.Start(time,1);   //for visualization
   }
   else
   {
      mPolyMgr.Stop(time, pitch);
      mVoiceParams.mAdsr.Stop(time);         //for visualization
      mVoiceParams.mFilterAdsr.Stop(time);   //for visualization
   }
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
   mPhaseOffsetSlider->Draw();
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
   
   {
      int x = 5;
      int y = 18;
      int height = 36;
      int width = 80;
      
      ofSetColor(100,100,.8f*gModuleDrawAlpha);
      ofSetLineWidth(.5f);
      ofRect(x,y,width,height, 0);
      
      ofSetColor(245, 58, 0, gModuleDrawAlpha);
      ofSetLineWidth(1);
      
      ofBeginShape();
      
      for (float i=0; i<width; i+=(.25f/gDrawScale))
      {
         float phase = i/width * FTWO_PI;
         phase += gTime * .005f;
         if (mDrawOsc.GetShuffle() > 0)
            phase *= 2;
         float value = mDrawOsc.Value(phase);
         ofVertex(i + x, ofMap(value,-1,1,0,height) + y);
      }
      ofEndShape(false);
   }
}

void SingleOscillator::GetModuleDimensions(int& width, int& height)
{
   width = 180;
   height = 125;
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
   if (list == mOscSelector)
      mDrawOsc.SetType(mVoiceParams.mOscType);
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
   if (slider == mFilterCutoffSlider)
      mFilterADSRDisplay->SetVol(mVoiceParams.mFilterCutoff / SINGLEOSCILLATOR_NO_CUTOFF);
   if (slider == mLengthMultiplierSlider)
   {
      mADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
      mFilterADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
   }
   if (slider == mShuffleSlider)
      mDrawOsc.SetShuffle(mVoiceParams.mShuffle);
   if (slider == mPulseWidthSlider)
      mDrawOsc.SetPulseWidth(mVoiceParams.mPulseWidth);
}

void SingleOscillator::IntSliderUpdated(IntSlider* slider, int oldVal)
{

}

void SingleOscillator::CheckboxUpdated(Checkbox* checkbox)
{
}


//
//  SignalGenerator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#include "SignalGenerator.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"
#include "FloatSliderLFOControl.h"

SignalGenerator::SignalGenerator()
: mOsc(kOsc_Sin)
, mVol(0)
, mSmoothedVol(0)
, mVolSlider(nullptr)
, mOscType(kOsc_Sin)
, mOscSelector(nullptr)
, mPulseWidth(.5f)
, mPulseWidthSlider(nullptr)
, mMult(1)
, mMultSelector(nullptr)
, mSync(false)
, mSyncCheckbox(nullptr)
, mSyncFreq(200)
, mSyncFreqSlider(nullptr)
, mDetune(1)
, mDetuneSlider(nullptr)
, mFreq(220)
, mFreqSlider(nullptr)
, mPhase(0)
, mSyncPhase(0)
, mFreqMode(kFreqMode_Instant)
, mFreqModeSelector(nullptr)
, mFreqSliderAmountSlider(nullptr)
, mFreqRampTime(200)
, mFreqRampTimeSlider(nullptr)
, mShuffle(0)
, mShuffleSlider(nullptr)
, mSoften(0)
, mSoftenSlider(nullptr)
, mPhaseOffset(0)
, mPhaseOffsetSlider(nullptr)
{
   mWriteBuffer = new float[gBufferSize];
   
   mOsc.Start(0,1);
}

void SignalGenerator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFreqSlider = new FloatSlider(this,"freq",5,2,170,15,&mFreq,1,4000);
   mFreqModeSelector = new DropdownList(this,"freq mode",5,21,(int*)(&mFreqMode));
   mOscSelector = new DropdownList(this,"osc",60,21,(int*)(&mOscType));
   mPulseWidthSlider = new FloatSlider(this,"pw",107,21,68,15,&mPulseWidth,0.01f,.99f);
   mFreqSliderAmountSlider = new FloatSlider(this,"slider",mFreqModeSelector,kAnchor_Below,80,15,&mFreqSliderAmount,0,1);
   mFreqRampTimeSlider = new FloatSlider(this,"ramp",mFreqModeSelector,kAnchor_Below,80,15,&mFreqRampTime,0,1000);
   mSyncCheckbox = new Checkbox(this,"sync",mFreqSliderAmountSlider,kAnchor_Right,&mSync);
   mSyncFreqSlider = new FloatSlider(this,"syncf",mSyncCheckbox,kAnchor_Right,40,15,&mSyncFreq,10,999.9f);
   mSoftenSlider = new FloatSlider(this,"soften",mFreqSliderAmountSlider,kAnchor_Below,80,15,&mSoften,0,1);
   mShuffleSlider = new FloatSlider(this,"shuffle",mSyncCheckbox,kAnchor_Below,80,15,&mShuffle,0, 1);
   mMultSelector = new DropdownList(this,"mult",mSoftenSlider,kAnchor_Below,&mMult);
   mPhaseOffsetSlider = new FloatSlider(this,"phase",mShuffleSlider,kAnchor_Below,80,15,&mPhaseOffset,0,1,3);
   mVolSlider = new FloatSlider(this,"vol",mMultSelector,kAnchor_Below,80,15,&mVol,0,1);
   mDetuneSlider = new FloatSlider(this,"detune",mPhaseOffsetSlider,kAnchor_Below,80,15,&mDetune,.98f,1.02f,3);
   
   mSyncFreqSlider->SetLabel("");
   
   SetFreqMode(kFreqMode_Instant);
   
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("noise",kOsc_Random);
   
   mMultSelector->AddLabel("1/8", -8);
   mMultSelector->AddLabel("1/7", -7);
   mMultSelector->AddLabel("1/6", -6);
   mMultSelector->AddLabel("1/5", -5);
   mMultSelector->AddLabel("1/4", -4);
   mMultSelector->AddLabel("1/3", -3);
   mMultSelector->AddLabel("1/2", -2);
   mMultSelector->AddLabel("1", 1);
   mMultSelector->AddLabel("2", 2);
   mMultSelector->AddLabel("3", 3);
   mMultSelector->AddLabel("4", 4);
   mMultSelector->AddLabel("5", 5);
   mMultSelector->AddLabel("6", 6);
   mMultSelector->AddLabel("7", 7);
   mMultSelector->AddLabel("8", 8);
   
   mFreqModeSelector->AddLabel("instant", kFreqMode_Instant);
   mFreqModeSelector->AddLabel("root", kFreqMode_Root);
   mFreqModeSelector->AddLabel("ramp", kFreqMode_Ramp);
   mFreqModeSelector->AddLabel("slider", kFreqMode_Slider);
   
   mFreqSlider->SetMode(FloatSlider::kSquare);
   mFreqRampTimeSlider->SetMode(FloatSlider::kSquare);
   
   mSoftenSlider->SetShowing(mOscType == kOsc_Square || mOscType == kOsc_Saw || mOscType == kOsc_NegSaw);
}

SignalGenerator::~SignalGenerator()
{
   delete[] mWriteBuffer;
}

void SignalGenerator::Process(double time)
{
   PROFILER(SignalGenerator);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   float syncPhaseInc = GetPhaseInc(mSyncFreq);
   for (int pos=0; pos<bufferSize; ++pos)
   {
      ComputeSliders(pos);
      
      float volSq;
      if (mVolSlider->GetLFO() && mVolSlider->GetLFO()->IsEnabled())
      {
         volSq = mVol * mVol;
      }
      else
      {
         float smooth = .001f;
         mSmoothedVol = mSmoothedVol * (1-smooth) + mVol * smooth;
         volSq = mSmoothedVol * mSmoothedVol;
      }
      
      if (mFreqMode == kFreqMode_Root)
         mFreq = TheScale->PitchToFreq(TheScale->ScaleRoot() + 24);
      else if (mFreqMode == kFreqMode_Ramp)
         mFreq = mFreqRamp.Value(time);
      else if (mFreqMode == kFreqMode_Slider)
         mFreq = ofLerp(mFreqSliderStart, mFreqSliderEnd, mFreqSliderAmount);
      
      float mult = mMult;
      if (mult < 0)
         mult = -1.0f/mult;
      float phaseInc = GetPhaseInc(mFreq * mDetune * mult);
      
      mPhase += phaseInc;
      if (mPhase == INFINITY)
      {
         ofLog() << "Infinite phase.";
      }
      else
      {
         while (mPhase > FTWO_PI*2)
         {
            mPhase -= FTWO_PI*2;
            mSyncPhase = 0;
         }
      }
      mSyncPhase += syncPhaseInc;
      
      if (mSync)
         mWriteBuffer[pos] += mOsc.Audio(time, mSyncPhase) * volSq;
      else
         mWriteBuffer[pos] += mOsc.Audio(time, mPhase + mPhaseOffset*FTWO_PI) * volSq;
      
      time += gInvSampleRateMs;
   }
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SignalGenerator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      if (mFreqMode == kFreqMode_Instant)
      {
         mFreq = TheScale->PitchToFreq(pitch);
      }
      else if (mFreqMode == kFreqMode_Ramp)
      {
         mFreqRamp.Start(time, TheScale->PitchToFreq(pitch), time+mFreqRampTime);
      }
      else if (mFreqMode == kFreqMode_Slider)
      {
         float freq = TheScale->PitchToFreq(pitch);
         if (freq >= mFreq)
         {
            mFreqSliderAmount = 0;
            mFreqSliderStart = mFreq;
            mFreqSliderEnd = freq;
         }
         else
         {
            mFreqSliderAmount = 1;
            mFreqSliderStart = freq;
            mFreqSliderEnd = mFreq;
         }
      }
   }
}

void SignalGenerator::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void SignalGenerator::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mPulseWidthSlider->Draw();
   mShuffleSlider->Draw();
   mMultSelector->Draw();
   mSyncCheckbox->Draw();
   mSyncFreqSlider->Draw();
   mOscSelector->Draw();
   mDetuneSlider->Draw();
   mFreqSlider->Draw();
   mFreqModeSelector->Draw();
   mFreqSliderAmountSlider->Draw();
   mFreqRampTimeSlider->Draw();
   mSoftenSlider->Draw();
   mPhaseOffsetSlider->Draw();
}

void SignalGenerator::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 108;
}

void SignalGenerator::SetType(OscillatorType type)
{
   mOscType = type;
   mOsc.SetType(type);
}

void SignalGenerator::SetFreqMode(SignalGenerator::FreqMode mode)
{
   mFreqMode = mode;
   mFreqSliderAmountSlider->SetShowing(mFreqMode == kFreqMode_Slider);
   mFreqRampTimeSlider->SetShowing(mFreqMode == kFreqMode_Ramp);
   if (mFreqMode == kFreqMode_Slider)
   {
      mFreqSliderAmount = 0;
      mFreqSliderStart = mFreq;
      mFreqSliderEnd = mFreq;
   }
   if (mFreqMode == kFreqMode_Ramp)
   {
      mFreqRamp.SetValue(mFreq);
   }
}

void SignalGenerator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("vol", moduleInfo, 0, mVolSlider);
   mModuleSaveData.LoadEnum<OscillatorType>("osc", moduleInfo, kOsc_Sin, mOscSelector);
   mModuleSaveData.LoadFloat("detune", moduleInfo, 1, mDetuneSlider);
   mModuleSaveData.LoadEnum<FreqMode>("freq_mode", moduleInfo, kFreqMode_Instant, mFreqModeSelector);
   
   SetUpFromSaveData();
}

void SignalGenerator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetVol(mModuleSaveData.GetFloat("vol"));
   SetType(mModuleSaveData.GetEnum<OscillatorType>("osc"));
   mDetune = mModuleSaveData.GetFloat("detune");
   SetFreqMode(mModuleSaveData.GetEnum<FreqMode>("freq_mode"));
}

void SignalGenerator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mOscSelector)
   {
      mOsc.SetType(mOscType);
      mSoftenSlider->SetShowing(mOscType == kOsc_Square || mOscType == kOsc_Saw || mOscType == kOsc_NegSaw);
   }
   if (list == mFreqModeSelector)
      SetFreqMode(mFreqMode);
}

void SignalGenerator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mPulseWidthSlider)
      mOsc.SetPulseWidth(mPulseWidth);
   if (slider == mShuffleSlider)
      mOsc.mOsc.SetShuffle(mShuffle);
   if (slider == mFreqSlider)
   {
      if (mFreqMode == kFreqMode_Ramp)
         mFreqRamp.Start(mFreq, mFreqRampTime);
   }
   if (slider == mSoftenSlider)
      mOsc.mOsc.SetSoften(mSoften);
}

void SignalGenerator::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   
}

void SignalGenerator::CheckboxUpdated(Checkbox* checkbox)
{
}


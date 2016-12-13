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

SignalGenerator::SignalGenerator()
: mOsc(kOsc_Sin)
, mVol(0)
, mSmoothedVol(0)
, mVolSlider(NULL)
, mOscType(kOsc_Sin)
, mOscSelector(NULL)
, mPulseWidth(.5f)
, mPulseWidthSlider(NULL)
, mMult(1)
, mMultSelector(NULL)
, mSync(false)
, mSyncCheckbox(NULL)
, mSyncFreq(200)
, mSyncFreqSlider(NULL)
, mDetune(1)
, mDetuneSlider(NULL)
, mFreq(220)
, mFreqSlider(NULL)
, mPhase(0)
, mSyncPhase(0)
, mFreqMode(kFreqMode_Instant)
, mFreqModeSelector(NULL)
, mFreqSliderAmountSlider(NULL)
, mFreqRampTime(200)
, mFreqRampTimeSlider(NULL)
, mShuffle(0)
, mShuffleSlider(NULL)
, mSoften(0)
, mSoftenSlider(NULL)
{
   mWriteBuffer = new float[gBufferSize];
   
   mOsc.Start(0,1);
}

void SignalGenerator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",5,72,80,15,&mVol,0,1);
   mOscSelector = new DropdownList(this,"osc",60,21,(int*)(&mOscType));
   mPulseWidthSlider = new FloatSlider(this,"pw",107,21,68,15,&mPulseWidth,0.01f,.99f);
   mShuffleSlider = new FloatSlider(this,"shuffle",95,55,80,15,&mShuffle,0, 1);
   mMultSelector = new DropdownList(this,"mult",5,55,&mMult);
   mSyncCheckbox = new Checkbox(this,"sync",95,38,&mSync);
   mSyncFreqSlider = new FloatSlider(this,"syncf",135,38,40,15,&mSyncFreq,10,999.9f);
   mDetuneSlider = new FloatSlider(this,"detune",95,72,80,15,&mDetune,.98f,1.02f,3);
   mFreqSlider = new FloatSlider(this,"freq",5,2,170,15,&mFreq,30,1000);
   mFreqModeSelector = new DropdownList(this,"freq mode",5,21,(int*)(&mFreqMode));
   mFreqSliderAmountSlider = new FloatSlider(this,"slider",5,38,80,15,&mFreqSliderAmount,0,1);
   mFreqRampTimeSlider = new FloatSlider(this,"ramp",5,38,80,15,&mFreqRampTime,0,1000);
   mSoftenSlider = new FloatSlider(this,"soften",-1,-1,40,15,&mSoften,0,1);
   
   mSyncFreqSlider->SetLabel("");
   mSoftenSlider->SetLabel("");
   mSoftenSlider->PositionTo(mMultSelector, kAnchorDirection_Right);
   
   SetFreqMode(kFreqMode_Instant);
   
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
   
   mFreqModeSelector->AddLabel("instant", kFreqMode_Instant);
   mFreqModeSelector->AddLabel("root", kFreqMode_Root);
   mFreqModeSelector->AddLabel("ramp", kFreqMode_Ramp);
   mFreqModeSelector->AddLabel("slider", kFreqMode_Slider);
   
   mFreqSlider->SetMode(FloatSlider::kLogarithmic);
   mFreqRampTimeSlider->SetMode(FloatSlider::kSquare);
}

SignalGenerator::~SignalGenerator()
{
   delete[] mWriteBuffer;
}

void SignalGenerator::Process(double time)
{
   Profiler profiler("SignalGenerator");
   
   if (!mEnabled || GetTarget() == NULL)
      return;
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   float syncPhaseInc = GetPhaseInc(mSyncFreq);
   for (int pos=0; pos<bufferSize; ++pos)
   {
      ComputeSliders(pos);
      
      float smooth = .001f;
      mSmoothedVol = mSmoothedVol * (1-smooth) + mVol * smooth;
      float volSq = mSmoothedVol * mSmoothedVol;
      
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
      while (mPhase > FTWO_PI*2)
      {
         mPhase -= FTWO_PI*2;
         mSyncPhase = 0;
      }
      mSyncPhase += syncPhaseInc;
      
      if (mSync)
         mWriteBuffer[pos] += mOsc.Audio(time, mSyncPhase) * volSq;
      else
         mWriteBuffer[pos] += mOsc.Audio(time, mPhase) * volSq;
      
      time += gInvSampleRateMs;
   }
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SignalGenerator::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
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
}

void SignalGenerator::GetModuleDimensions(int& width, int& height)
{
   width = 180;
   height = 90;
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
      mOsc.SetType(mOscType);
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


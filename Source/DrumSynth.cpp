//
//  DrumSynth.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 8/5/14.
//
//

#include "DrumSynth.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "ModularSynth.h"
#include "MidiController.h"
#include "Profiler.h"

DrumSynth::DrumSynth()
: mVolume(1)
, mVolSlider(nullptr)
, mEditMode(false)
, mEditCheckbox(nullptr)
, mCurrentEditHit(3)
{
   mOutputBuffer = new float[gBufferSize];
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      int x = i%4*PAD_DRAW_SIZE + 5;
      int y = (3-(i/4))*PAD_DRAW_SIZE + 70;
      mHits[i] = new DrumSynthHit(this, i,  x, y);
   }
}

void DrumSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",4,4,100,15,&mVolume,0,2);
   mEditCheckbox = new Checkbox(this,"edit",73,20,&mEditMode);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mHits[i]->CreateUIControls();
}

DrumSynth::~DrumSynth()
{
   delete[] mOutputBuffer;
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      delete mHits[i];
}

void DrumSynth::Process(double time)
{
   PROFILER(DrumSynth);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   float volSq = mVolume * mVolume;
   
   Clear(mOutputBuffer, bufferSize);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      mHits[i]->Process(time, mOutputBuffer, bufferSize);
   }
   
   Mult(mOutputBuffer, volSq, bufferSize);
   
   GetVizBuffer()->WriteChunk(mOutputBuffer, bufferSize, 0);
   
   Add(out, mOutputBuffer, bufferSize);
}

void DrumSynth::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (pitch >= 0 && pitch < NUM_DRUM_HITS)
   {
      if (velocity > 0)
         mHits[pitch]->Play(time, velocity/127.0f);
   }
}

void DrumSynth::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (!mEditMode)
      return;
   
   x -= 5;
   y -= 70;
   if (x<0 || y<0)
      return;
   x /= PAD_DRAW_SIZE;
   y /= PAD_DRAW_SIZE;
   if (x < 4 && y < 4)
   {
      int sampleIdx = GetAssociatedSampleIndex(x, y);
      if (sampleIdx != -1)
      {
         //mHits[sampleIdx]->Play(gTime);
         //mVelocity[sampleIdx] = 1;
      }
   }
}

void DrumSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mEditCheckbox->Draw();
   
   if (mEditMode)
   {
      ofPushMatrix();
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         ofPushStyle();
         if (mHits[i]->Level() > 0)
         {
            ofFill();
            ofSetColor(200,100,0,gModuleDrawAlpha * sqrtf(mHits[i]->Level()));
            ofRect(mHits[i]->mX,mHits[i]->mY,PAD_DRAW_SIZE,PAD_DRAW_SIZE);
         }
         ofSetColor(200,100,0,gModuleDrawAlpha);
         ofNoFill();
         ofRect(mHits[i]->mX,mHits[i]->mY,PAD_DRAW_SIZE,PAD_DRAW_SIZE);
         ofPopStyle();
         
         ofSetColor(255,255,255,gModuleDrawAlpha);
         
         string name = ofToString(i);
         DrawTextNormal(name,mHits[i]->mX+5,mHits[i]->mY+10);
         
         mHits[i]->Draw();
      }
      ofPopMatrix();
   }
}

int DrumSynth::GetAssociatedSampleIndex(int x, int y)
{
   int pos = x+(3-y)*4;
   if (pos < 16)
      return pos;
   return -1;
}

void DrumSynth::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = 600;
      height = 670;
   }
   else
   {
      width = 110;
      height = 40;
   }
}

void DrumSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void DrumSynth::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void DrumSynth::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void DrumSynth::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEditCheckbox)
   {
      TheSynth->MoveToFront(this);
   }
}

void DrumSynth::ButtonClicked(ClickButton *button)
{
}

void DrumSynth::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void DrumSynth::TextEntryComplete(TextEntry* entry)
{
}

void DrumSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void DrumSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

DrumSynth::DrumSynthHit::DrumSynthHit(DrumSynth* parent, int index, int x, int y)
: mPhase(0)
, mVolSlider(nullptr)
, mFreqMaxSlider(nullptr)
, mFreqMinSlider(nullptr)
, mToneType(nullptr)
, mToneAdsrDisplay(nullptr)
, mFreqAdsrDisplay(nullptr)
, mVolNoiseSlider(nullptr)
, mNoiseAdsrDisplay(nullptr)
, mParent(parent)
, mIndex(index)
, mX(x)
, mY(y)
{
}

void DrumSynth::DrumSynthHit::CreateUIControls()
{
   mVolSlider = new FloatSlider(mParent, ("vol"+ofToString(mIndex)).c_str(), mX+5, mY+55, 60,15,&mData.mVol,0,1);
   mFreqMaxSlider = new FloatSlider(mParent, ("fmax" + ofToString(mIndex)).c_str(), mX + 35, mY + 105, 100, 15, &mData.mFreqMax, 0, 1600);
   mFreqMinSlider = new FloatSlider(mParent, ("fmin" + ofToString(mIndex)).c_str(), mX + 35, mY + 120, 100, 15, &mData.mFreqMin, 0, 1600);
   mToneType = new RadioButton(mParent, ("type"+ofToString(mIndex)).c_str(), mX+5, mY+75,(int*)(&mData.mTone.mOsc.mType));
   mToneAdsrDisplay = new ADSRDisplay(mParent, ("adsrtone"+ofToString(mIndex)).c_str(), mX+5, mY+15, 60,40,mData.mTone.GetADSR());
   mFreqAdsrDisplay = new ADSRDisplay(mParent, ("adsrfreq"+ofToString(mIndex)).c_str(), mX+35, mY+75,100,29,&mData.mFreqAdsr);
   mVolNoiseSlider = new FloatSlider(mParent, ("noise"+ofToString(mIndex)).c_str(), mX+70, mY+55,60,15,&mData.mVolNoise,0,1,2);
   mNoiseAdsrDisplay = new ADSRDisplay(mParent, ("adsrnoise"+ofToString(mIndex)).c_str(), mX+70, mY+15,60,40,mData.mNoise.GetADSR());
   
   mToneType->AddLabel("sin",kOsc_Sin);
   mToneType->AddLabel("saw",kOsc_Saw);
   mToneType->AddLabel("squ",kOsc_Square);
   mToneType->AddLabel("tri",kOsc_Tri);
   
   mFreqAdsrDisplay->SetMaxTime(500);
   mToneAdsrDisplay->SetMaxTime(500);
   mNoiseAdsrDisplay->SetMaxTime(500);
   
   mFreqMaxSlider->SetMode(FloatSlider::kSquare);
   mFreqMinSlider->SetMode(FloatSlider::kSquare);
}

void DrumSynth::DrumSynthHit::Play(double time, float velocity)
{
   float envelopeScale = ofLerp(.2f, 1, velocity);
   mData.mFreqAdsr.Start(time, 1, envelopeScale);
   mData.mTone.GetADSR()->Start(time, velocity, envelopeScale);
   mData.mNoise.GetADSR()->Start(time,velocity, envelopeScale);
   mStartTime = time;
}

void DrumSynth::DrumSynthHit::Process(double time, float* out, int bufferSize)
{
   if (mData.mTone.GetADSR()->IsDone(time) && mData.mNoise.GetADSR()->IsDone(time))
   {
      mLevel.Reset();
      return;
   }
   
   for (int i=0; i<bufferSize; ++i)
   {
      float freq = ofLerp(mData.mFreqMin, mData.mFreqMax, mData.mFreqAdsr.Value(time));
      float phaseInc = GetPhaseInc(freq);
      
      float sample = mData.mTone.Audio(time, mPhase) * mData.mVol * mData.mVol;
      float noise = mData.mNoise.Audio(time, mPhase);
      noise *= noise * (noise > 0 ? 1 : -1); //square but keep sign
      sample += noise * mData.mVolNoise * mData.mVolNoise;
      mLevel.Process(&sample, 1);
      out[i] += sample;
      
      mPhase += phaseInc;
      while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }
      
      time += gInvSampleRateMs;
   }
}

void DrumSynth::DrumSynthHit::Draw()
{
   ofSetColor(255,0,0);
   ofRect(mToneAdsrDisplay->GetRect(true));
   ofSetColor(0,255,0);
   ofRect(mNoiseAdsrDisplay->GetRect(true));
   ofSetColor(0,0,255);
   ofRect(mFreqAdsrDisplay->GetRect(true));
   
   mToneAdsrDisplay->Draw();
   mVolSlider->Draw();
   mNoiseAdsrDisplay->Draw();
   mVolNoiseSlider->Draw();
   mToneType->Draw();
   mFreqAdsrDisplay->Draw();
   mFreqMaxSlider->Draw();
   mFreqMinSlider->Draw();
   
   float time = gTime - mStartTime;
   if (time >= 0 && time < 500)
   {
      ofPushStyle();
      ofSetColor(255,0,0);
      ofRect(mX+(time/500)*140,mY,1,140);
      ofPopStyle();
   }
}

DrumSynth::DrumSynthHitSerialData::DrumSynthHitSerialData()
: mTone(kOsc_Sin)
, mNoise(kOsc_Random)
, mFreqMax(150)
, mFreqMin(10)
, mVol(0)
, mVolNoise(0)
{
   mTone.GetADSR()->SetNumStages(2);
   mTone.GetADSR()->GetHasSustainStage() = false;
   mTone.GetADSR()->GetStageData(0).time = 1;
   mTone.GetADSR()->GetStageData(0).target = 1;
   mTone.GetADSR()->GetStageData(1).time = 100;
   mTone.GetADSR()->GetStageData(1).target = 0;
   mNoise.GetADSR()->SetNumStages(2);
   mNoise.GetADSR()->GetHasSustainStage() = false;
   mNoise.GetADSR()->GetStageData(0).time = 1;
   mNoise.GetADSR()->GetStageData(0).target = 1;
   mNoise.GetADSR()->GetStageData(1).time = 40;
   mNoise.GetADSR()->GetStageData(1).target = 0;
   mFreqAdsr.SetNumStages(2);
   mFreqAdsr.GetHasSustainStage() = false;
   mFreqAdsr.GetFreeReleaseLevel() = true;
   mFreqAdsr.GetStageData(0).time = 1;
   mFreqAdsr.GetStageData(0).target = 1;
   mFreqAdsr.GetStageData(1).time = 500;
   mFreqAdsr.GetStageData(1).target = 0;
}




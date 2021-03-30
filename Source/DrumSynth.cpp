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
#include "UIControlMacros.h"

#define DRUMSYNTH_NO_CUTOFF 10000

DrumSynth::DrumSynth()
: mVolume(1)
, mVolSlider(nullptr)
, mEditMode(false)
, mEditCheckbox(nullptr)
{
   mOutputBuffer = new float[gBufferSize];
   
   for (size_t i=0; i<mHits.size(); ++i)
   {
      int x = (i % DRUMSYNTH_PADS_HORIZONTAL)*DRUMSYNTH_PAD_WIDTH + 5;
      int y = (1-(i/ DRUMSYNTH_PADS_HORIZONTAL))*DRUMSYNTH_PAD_HEIGHT + 50;
      mHits[i] = new DrumSynthHit(this, i,  x, y);
   }
}

void DrumSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",4,4,100,15,&mVolume,0,2);
   mEditCheckbox = new Checkbox(this,"edit",73,20,&mEditMode);
   
   for (size_t i=0; i<mHits.size(); ++i)
      mHits[i]->CreateUIControls();
}

DrumSynth::~DrumSynth()
{
   delete[] mOutputBuffer;
   for (size_t i=0; i<mHits.size(); ++i)
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
   
   for (size_t i=0; i<mHits.size(); ++i)
   {
      mHits[i]->Process(time, mOutputBuffer, bufferSize);
   }
   
   Mult(mOutputBuffer, volSq, bufferSize);
   
   GetVizBuffer()->WriteChunk(mOutputBuffer, bufferSize, 0);
   
   Add(out, mOutputBuffer, bufferSize);
}

void DrumSynth::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (pitch >= 0 && pitch < mHits.size())
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
   y -= 50;
   if (x<0 || y<0)
      return;
   x /= DRUMSYNTH_PAD_WIDTH;
   y /= DRUMSYNTH_PAD_HEIGHT;
   if (x < DRUMSYNTH_PADS_HORIZONTAL && y < DRUMSYNTH_PADS_VERTICAL)
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
      for (size_t i=0; i<mHits.size(); ++i)
      {
         ofPushStyle();
         if (mHits[i]->Level() > 0)
         {
            ofFill();
            ofSetColor(200,100,0,gModuleDrawAlpha * sqrtf(mHits[i]->Level()));
            ofRect(mHits[i]->mX,mHits[i]->mY, DRUMSYNTH_PAD_WIDTH, DRUMSYNTH_PAD_HEIGHT);
         }
         ofSetColor(200,100,0,gModuleDrawAlpha);
         ofNoFill();
         ofRect(mHits[i]->mX,mHits[i]->mY, DRUMSYNTH_PAD_WIDTH, DRUMSYNTH_PAD_HEIGHT);
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
   int pos = x+(DRUMSYNTH_PADS_VERTICAL-1-y)*DRUMSYNTH_PADS_HORIZONTAL;
   if (pos < DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL)
      return pos;
   return -1;
}

void DrumSynth::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = 10 + MIN(mHits.size(), DRUMSYNTH_PADS_HORIZONTAL) * DRUMSYNTH_PAD_WIDTH;
      height = 52 + mHits.size() / DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PAD_HEIGHT;
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
   mFilter.SetFilterType(kFilterType_Lowpass);
   mFilter.SetFilterParams(1000, 1);
}

void DrumSynth::DrumSynthHit::CreateUIControls()
{
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER mParent //change owner

   float width, height;
   float kColumnWidth = (DRUMSYNTH_PAD_WIDTH-5*2-3) * .5f;
   UIBLOCK(mX + 5, mY + 15, kColumnWidth);

   UICONTROL_CUSTOM(mToneAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrtone" + ofToString(mIndex)).c_str()), kColumnWidth, 36, mData.mTone.GetADSR()));
   FLOATSLIDER(mVolSlider, ("vol" + ofToString(mIndex)).c_str(), &mData.mVol, 0, 1);

   UIBLOCK_NEWCOLUMN();
   UICONTROL_CUSTOM(mNoiseAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrnoise" + ofToString(mIndex)).c_str()), kColumnWidth, 36, mData.mNoise.GetADSR()));
   FLOATSLIDER(mVolNoiseSlider, ("noise" + ofToString(mIndex)).c_str(), &mData.mVolNoise, 0, 1, 2);
   ENDUIBLOCK(width, height);

   UIBLOCK(mX + 5, height+3);
   UICONTROL_CUSTOM(mToneType, new RadioButton(UICONTROL_BASICS(("type" + ofToString(mIndex)).c_str()), (int*)(&mData.mTone.mOsc.mType)));
   UIBLOCK_SHIFTX(30);
   float freqAdsrWidth = DRUMSYNTH_PAD_WIDTH - 5 * 2 - 3 - 30;
   UICONTROL_CUSTOM(mFreqAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrfreq" + ofToString(mIndex)).c_str()), freqAdsrWidth, 36, &mData.mFreqAdsr));
   UIBLOCK_PUSHSLIDERWIDTH(freqAdsrWidth);
   FLOATSLIDER(mFreqMaxSlider, ("freqmax" + ofToString(mIndex)).c_str(), &mData.mFreqMax, 0, 1600);
   FLOATSLIDER(mFreqMinSlider, ("freqmin" + ofToString(mIndex)).c_str(), &mData.mFreqMin, 0, 1600);   

   UIBLOCK_NEWLINE();
   float filterAdsrWidth = DRUMSYNTH_PAD_WIDTH - 5 * 2;
   UICONTROL_CUSTOM(mFilterAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrfilter" + ofToString(mIndex)).c_str()), filterAdsrWidth, 36, &mData.mFilterAdsr));
   UIBLOCK_PUSHSLIDERWIDTH(filterAdsrWidth);
   FLOATSLIDER(mFilterCutoffMaxSlider, ("cutoffmax" + ofToString(mIndex)).c_str(), &mData.mCutoffMax, 10, DRUMSYNTH_NO_CUTOFF);
   FLOATSLIDER(mFilterCutoffMinSlider, ("cutoffmin" + ofToString(mIndex)).c_str(), &mData.mCutoffMin, 10, DRUMSYNTH_NO_CUTOFF);
   FLOATSLIDER(mFilterQSlider, ("q" + ofToString(mIndex)).c_str(), &mData.mQ, .1, 20);

   ENDUIBLOCK0();

#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset
   
   mToneType->AddLabel("sin",kOsc_Sin);
   mToneType->AddLabel("saw",kOsc_Saw);
   mToneType->AddLabel("squ",kOsc_Square);
   mToneType->AddLabel("tri",kOsc_Tri);
   
   mFreqAdsrDisplay->SetMaxTime(500);
   mToneAdsrDisplay->SetMaxTime(500);
   mNoiseAdsrDisplay->SetMaxTime(500);
   mFilterAdsrDisplay->SetMaxTime(500);
   
   mFreqMaxSlider->SetMode(FloatSlider::kSquare);
   mFreqMinSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMaxSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMinSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMaxSlider->SetMaxValueDisplay("none");
}

void DrumSynth::DrumSynthHit::Play(double time, float velocity)
{
   float envelopeScale = ofLerp(.2f, 1, velocity);
   mData.mFreqAdsr.Start(time, 1, envelopeScale);
   mData.mFilterAdsr.Start(time, 1, envelopeScale);
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
   
   for (size_t i=0; i<bufferSize; ++i)
   {
      float freq = ofLerp(mData.mFreqMin, mData.mFreqMax, mData.mFreqAdsr.Value(time));
      if (mData.mCutoffMax != DRUMSYNTH_NO_CUTOFF)
         mFilter.SetFilterParams(ofLerp(mData.mCutoffMin, mData.mCutoffMax, mData.mFilterAdsr.Value(time)), mData.mQ);
      float phaseInc = GetPhaseInc(freq);
      
      float sample = mData.mTone.Audio(time, mPhase) * mData.mVol * mData.mVol;
      float noise = mData.mNoise.Audio(time, mPhase);
      noise *= noise * (noise > 0 ? 1 : -1); //square but keep sign
      sample += noise * mData.mVolNoise * mData.mVolNoise;
      if (mData.mCutoffMax != DRUMSYNTH_NO_CUTOFF)
         sample = mFilter.Filter(sample);
      mLevel.Process(&sample, 1);      
      out[i] += sample;
      
      mPhase += phaseInc;
      while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }
      
      time += gInvSampleRateMs;
   }
}

void DrumSynth::DrumSynthHit::Draw()
{
   /*ofSetColor(255,0,0);
   ofRect(mToneAdsrDisplay->GetRect(true));
   ofSetColor(0,255,0);
   ofRect(mNoiseAdsrDisplay->GetRect(true));
   ofSetColor(0,0,255);
   ofRect(mFreqAdsrDisplay->GetRect(true));*/
   
   mToneAdsrDisplay->Draw();
   mVolSlider->Draw();
   mNoiseAdsrDisplay->Draw();
   mVolNoiseSlider->Draw();
   mToneType->Draw();
   mFreqAdsrDisplay->Draw();
   mFreqMaxSlider->Draw();
   mFreqMinSlider->Draw();
   mFilterAdsrDisplay->Draw();
   mFilterCutoffMaxSlider->Draw();
   mFilterCutoffMinSlider->Draw();
   mFilterQSlider->Draw();

   ofPushStyle();
   ofSetColor(0, 0, 0, 100);
   ofFill();
   if (mData.mVol == 0)
   {
      ofRect(mToneAdsrDisplay->GetRect(true).grow(1));
      ofRect(mFreqAdsrDisplay->GetRect(true).grow(1));
      ofRect(mFreqMaxSlider->GetRect(true));
      ofRect(mFreqMinSlider->GetRect(true));
      ofRect(mToneType->GetRect(true));
   }
   if (mData.mVolNoise == 0)
   {
      ofRect(mNoiseAdsrDisplay->GetRect(true).grow(1));
   }
   if (mData.mVol == 0 && mData.mVolNoise == 0)
   {
      ofRect(mFilterCutoffMaxSlider->GetRect(true));
   }
   if (mData.mCutoffMax == DRUMSYNTH_NO_CUTOFF || (mData.mVol == 0 && mData.mVolNoise == 0))
   {
      ofRect(mFilterAdsrDisplay->GetRect(true).grow(1));
      ofRect(mFilterCutoffMinSlider->GetRect(true));
      ofRect(mFilterQSlider->GetRect(true));  
   }
   ofPopStyle();
}

DrumSynth::DrumSynthHitSerialData::DrumSynthHitSerialData()
: mTone(kOsc_Sin)
, mNoise(kOsc_Random)
, mFreqMax(150)
, mFreqMin(10)
, mVol(0)
, mVolNoise(0)
, mCutoffMax(DRUMSYNTH_NO_CUTOFF)
, mCutoffMin(10)
, mQ(1)
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

   mFilterAdsr.SetNumStages(2);
   mFilterAdsr.GetHasSustainStage() = false;
   mFilterAdsr.GetFreeReleaseLevel() = true;
   mFilterAdsr.GetStageData(0).time = 1;
   mFilterAdsr.GetStageData(0).target = 1;
   mFilterAdsr.GetStageData(1).time = 500;
   mFilterAdsr.GetStageData(1).target = 0;
}




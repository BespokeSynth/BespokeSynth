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
#include "FillSaveDropdown.h"

DrumSynth::DrumSynth()
: mVolume(1)
, mLoadedKit(0)
, mVolSlider(nullptr)
, mKitSelector(nullptr)
, mEditMode(false)
, mEditCheckbox(nullptr)
, mSaveButton(nullptr)
, mNewKitButton(nullptr)
, mCurrentEditHit(3)
, mNewKitNameEntry(nullptr)
{
   ReadKits();

   mOutputBuffer = new float[gBufferSize];
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      int x = i%4*PAD_DRAW_SIZE + 5;
      int y = (3-(i/4))*PAD_DRAW_SIZE + 70;
      mHits[i] = new DrumSynthHit(this, i,  x, y);
   }
   
   strcpy(mNewKitName, "new");
}

void DrumSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",4,4,100,15,&mVolume,0,2);
   mKitSelector = new DropdownList(this,"kit",4,20,&mLoadedKit);
   mEditCheckbox = new Checkbox(this,"edit",73,20,&mEditMode);
   mSaveButton = new ClickButton(this,"save current",200,22);
   mNewKitButton = new ClickButton(this,"new kit", 200, 4);
   mNewKitNameEntry = new TextEntry(this,"kitname", 130,4,7,mNewKitName);
   
   for (int i=0; i<mKits.size(); ++i)
      mKitSelector->AddLabel(mKits[i].mName.c_str(), i);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mHits[i]->CreateUIControls();
}

DrumSynth::~DrumSynth()
{
   delete[] mOutputBuffer;
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      delete mHits[i];
}

void DrumSynth::LoadKit(int kit)
{
   if (kit >= 0 && kit < mKits.size())
   {
      mLoadedKit = kit;
      
      for (int j=0; j<NUM_DRUM_HITS; ++j)
      {
         mHits[j]->mData.mTone.mOsc.mType = mKits[kit].mHits[j].mTone.mOsc.mType;
         mHits[j]->mData.mVol = mKits[kit].mHits[j].mVol;
         mHits[j]->mData.mTone.GetADSR()->GetA() = mKits[kit].mHits[j].mTone.GetADSR()->GetA();
         mHits[j]->mData.mTone.GetADSR()->GetD() = mKits[kit].mHits[j].mTone.GetADSR()->GetD();
         mHits[j]->mData.mTone.GetADSR()->GetS() = mKits[kit].mHits[j].mTone.GetADSR()->GetS();
         mHits[j]->mData.mTone.GetADSR()->GetR() = mKits[kit].mHits[j].mTone.GetADSR()->GetR();
         mHits[j]->mData.mVolNoise = mKits[kit].mHits[j].mVolNoise;
         mHits[j]->mData.mNoise.GetADSR()->GetA() = mKits[kit].mHits[j].mNoise.GetADSR()->GetA();
         mHits[j]->mData.mNoise.GetADSR()->GetD() = mKits[kit].mHits[j].mNoise.GetADSR()->GetD();
         mHits[j]->mData.mNoise.GetADSR()->GetS() = mKits[kit].mHits[j].mNoise.GetADSR()->GetS();
         mHits[j]->mData.mNoise.GetADSR()->GetR() = mKits[kit].mHits[j].mNoise.GetADSR()->GetR();
         mHits[j]->mData.mFreq = mKits[kit].mHits[j].mFreq;
         mHits[j]->mData.mFreqAdsr.GetA() = mKits[kit].mHits[j].mFreqAdsr.GetA();
         mHits[j]->mData.mFreqAdsr.GetD() = mKits[kit].mHits[j].mFreqAdsr.GetD();
         mHits[j]->mData.mFreqAdsr.GetS() = mKits[kit].mHits[j].mFreqAdsr.GetS();
         mHits[j]->mData.mFreqAdsr.GetR() = mKits[kit].mHits[j].mFreqAdsr.GetR();
      }
   }
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
   mKitSelector->Draw();
   mEditCheckbox->Draw();
   
   if (mEditMode)
   {
      mSaveButton->Draw();
      mNewKitButton->Draw();
      mNewKitNameEntry->Draw();
      
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

void DrumSynth::OnMidiNote(MidiNote& note)
{
}

void DrumSynth::OnMidiControl(MidiControl& control)
{
   int c = control.mControl;
   if (c >= 32 && c < 48)
   {
      if (control.mChannel == 1)
      {
         float value = control.mValue / 127.0f;
         if (c == 32)
            mHits[mCurrentEditHit]->mVolSlider->SetFromMidiCC(value);
         else if (c == 33)
            mHits[mCurrentEditHit]->mVolNoiseSlider->SetFromMidiCC(value);
         else if (c == 34)
            mHits[mCurrentEditHit]->mToneType->SetFromMidiCC(value);
         else if (c == 35)
            mHits[mCurrentEditHit]->mFreqSlider->SetFromMidiCC(value);
         else
         {
            int row = (c - 32) / 4;
            int col = (c - 32) % 4;
            
            ::ADSR* adsr;
            
            if (row == 1)
               adsr = mHits[mCurrentEditHit]->mData.mTone.GetADSR();
            else if (row == 2)
               adsr = mHits[mCurrentEditHit]->mData.mNoise.GetADSR();
            else
               adsr = &mHits[mCurrentEditHit]->mData.mFreqAdsr;
            
            if (col == 0)
               adsr->GetA() = value * 300 + 1;
            else if (col == 1)
               adsr->GetD() = value * 300;
            else if (col == 2)
               adsr->GetS() = value;
            else
               adsr->GetR() = value * 300 + 1;
         }
         
      }
      else if (control.mChannel == 2)
      {
         int pos = 47 - c;
         int x = 3 - pos % 4;
         int y = 3 - pos / 4;
         int index = GetAssociatedSampleIndex(x, y);
         
         if (index >= 0 && index < NUM_DRUM_HITS)
         {
            mCurrentEditHit = index;
            
            mTwister->SendCC(0, 32, mHits[mCurrentEditHit]->mVolSlider->GetMidiValue()*127);
            mTwister->SendCC(0, 33, mHits[mCurrentEditHit]->mVolNoiseSlider->GetMidiValue()*127);
            mTwister->SendCC(0, 34, mHits[mCurrentEditHit]->mToneType->GetMidiValue()*127);
            mTwister->SendCC(0, 35, mHits[mCurrentEditHit]->mFreqSlider->GetMidiValue()*127);
            for (int i=0; i<3; ++i)
            {
               ::ADSR* adsr;
               if (i == 0)
                  adsr = mHits[mCurrentEditHit]->mData.mTone.GetADSR();
               else if (i == 1)
                  adsr = mHits[mCurrentEditHit]->mData.mNoise.GetADSR();
               else
                  adsr = &mHits[mCurrentEditHit]->mData.mFreqAdsr;
               
               mTwister->SendCC(0, 36+i*4, adsr->GetA()/300*127);
               mTwister->SendCC(0, 37+i*4, adsr->GetD()/300*127);
               mTwister->SendCC(0, 38+i*4, adsr->GetS()*127);
               mTwister->SendCC(0, 39+i*4, adsr->GetR()/300*127);
            }
         }
      }
   }
}

int DrumSynth::GetAssociatedSampleIndex(int x, int y)
{
   int pos = x+(3-y)*4;
   if (pos < 16)
      return pos;
   return -1;
}

void DrumSynth::SaveKits()
{
   ofxJSONElement root;
   
   Json::Value& kits = root["kits"];
   for (int i=0; i<mKits.size(); ++i)
   {
      Json::Value& kit = kits[i];
      
      if (i == mLoadedKit)
      {
         for (int j=0; j<NUM_DRUM_HITS; ++j)
         {
            mKits[i].mHits[j].mTone.mOsc.mType = mHits[j]->mData.mTone.mOsc.mType;
            mKits[i].mHits[j].mVol = mHits[j]->mData.mVol;
            mKits[i].mHits[j].mTone.GetADSR()->GetA() = mHits[j]->mData.mTone.GetADSR()->GetA();
            mKits[i].mHits[j].mTone.GetADSR()->GetD() = mHits[j]->mData.mTone.GetADSR()->GetD();
            mKits[i].mHits[j].mTone.GetADSR()->GetS() = mHits[j]->mData.mTone.GetADSR()->GetS();
            mKits[i].mHits[j].mTone.GetADSR()->GetR() = mHits[j]->mData.mTone.GetADSR()->GetR();
            mKits[i].mHits[j].mVolNoise = mHits[j]->mData.mVolNoise;
            mKits[i].mHits[j].mNoise.GetADSR()->GetA() = mHits[j]->mData.mNoise.GetADSR()->GetA();
            mKits[i].mHits[j].mNoise.GetADSR()->GetD() = mHits[j]->mData.mNoise.GetADSR()->GetD();
            mKits[i].mHits[j].mNoise.GetADSR()->GetS() = mHits[j]->mData.mNoise.GetADSR()->GetS();
            mKits[i].mHits[j].mNoise.GetADSR()->GetR() = mHits[j]->mData.mNoise.GetADSR()->GetR();
            mKits[i].mHits[j].mFreq = mHits[j]->mData.mFreq;
            mKits[i].mHits[j].mFreqAdsr.GetA() = mHits[j]->mData.mFreqAdsr.GetA();
            mKits[i].mHits[j].mFreqAdsr.GetD() = mHits[j]->mData.mFreqAdsr.GetD();
            mKits[i].mHits[j].mFreqAdsr.GetS() = mHits[j]->mData.mFreqAdsr.GetS();
            mKits[i].mHits[j].mFreqAdsr.GetR() = mHits[j]->mData.mFreqAdsr.GetR();
         }
      }
      
      for (int j=0; j<NUM_DRUM_HITS; ++j)
      {
         kit["hits"][j]["tonetype"] = mKits[i].mHits[j].mTone.mOsc.mType;
         kit["hits"][j]["tonevol"] = mKits[i].mHits[j].mVol;
         kit["hits"][j]["tone_a"] = mKits[i].mHits[j].mTone.GetADSR()->GetA();
         kit["hits"][j]["tone_d"] = mKits[i].mHits[j].mTone.GetADSR()->GetD();
         kit["hits"][j]["tone_s"] = mKits[i].mHits[j].mTone.GetADSR()->GetS();
         kit["hits"][j]["tone_r"] = mKits[i].mHits[j].mTone.GetADSR()->GetR();
         kit["hits"][j]["noisevol"] = mKits[i].mHits[j].mVolNoise;
         kit["hits"][j]["noise_a"] = mKits[i].mHits[j].mNoise.GetADSR()->GetA();
         kit["hits"][j]["noise_d"] = mKits[i].mHits[j].mNoise.GetADSR()->GetD();
         kit["hits"][j]["noise_s"] = mKits[i].mHits[j].mNoise.GetADSR()->GetS();
         kit["hits"][j]["noise_r"] = mKits[i].mHits[j].mNoise.GetADSR()->GetR();
         kit["hits"][j]["freq"] = mKits[i].mHits[j].mFreq;
         kit["hits"][j]["freq_a"] = mKits[i].mHits[j].mFreqAdsr.GetA();
         kit["hits"][j]["freq_d"] = mKits[i].mHits[j].mFreqAdsr.GetD();
         kit["hits"][j]["freq_s"] = mKits[i].mHits[j].mFreqAdsr.GetS();
         kit["hits"][j]["freq_r"] = mKits[i].mHits[j].mFreqAdsr.GetR();
      }
      kit["name"] = mKits[i].mName;
   }
   
   root.save(ofToDataPath("drums/drumsynth.json"), true);
}

void DrumSynth::ReadKits()
{
   ofxJSONElement root;
   root.open(ofToDataPath("drums/drumsynth.json"));
   
   Json::Value& kits = root["kits"];
   mKits.resize(kits.size());
   for (int i=0; i<kits.size(); ++i)
   {
      Json::Value& kit = kits[i];
      for (int j=0; j<NUM_DRUM_HITS; ++j)
      {
         mKits[i].mHits[j].mTone.mOsc.mType = (OscillatorType)kit["hits"][j]["tonetype"].asInt();
         mKits[i].mHits[j].mVol = kit["hits"][j]["tonevol"].asDouble();
         mKits[i].mHits[j].mTone.GetADSR()->GetA() = kit["hits"][j]["tone_a"].asDouble();
         mKits[i].mHits[j].mTone.GetADSR()->GetD() = kit["hits"][j]["tone_d"].asDouble();
         mKits[i].mHits[j].mTone.GetADSR()->GetS() = kit["hits"][j]["tone_s"].asDouble();
         mKits[i].mHits[j].mTone.GetADSR()->GetR() = kit["hits"][j]["tone_r"].asDouble();
         mKits[i].mHits[j].mVolNoise = kit["hits"][j]["noisevol"].asDouble();
         mKits[i].mHits[j].mNoise.GetADSR()->GetA() = kit["hits"][j]["noise_a"].asDouble();
         mKits[i].mHits[j].mNoise.GetADSR()->GetD() = kit["hits"][j]["noise_d"].asDouble();
         mKits[i].mHits[j].mNoise.GetADSR()->GetS() = kit["hits"][j]["noise_s"].asDouble();
         mKits[i].mHits[j].mNoise.GetADSR()->GetR() = kit["hits"][j]["noise_r"].asDouble();
         mKits[i].mHits[j].mFreq = kit["hits"][j]["freq"].asDouble();
         mKits[i].mHits[j].mFreqAdsr.GetA() = kit["hits"][j]["freq_a"].asDouble();
         mKits[i].mHits[j].mFreqAdsr.GetD() = kit["hits"][j]["freq_d"].asDouble();
         mKits[i].mHits[j].mFreqAdsr.GetS() = kit["hits"][j]["freq_s"].asDouble();
         mKits[i].mHits[j].mFreqAdsr.GetR() = kit["hits"][j]["freq_r"].asDouble();
      }
      mKits[i].mName = kit["name"].asString();
   }
}

void DrumSynth::CreateKit()
{
   StoredDrumKit kit;
   
   kit.mName = mNewKitName;
   mKits.push_back(kit);
   mLoadedKit = (int)mKits.size() - 1;
   mKitSelector->AddLabel(kit.mName.c_str(), mLoadedKit);
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
   if (list == mKitSelector)
      LoadKit(mLoadedKit);
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
   if (button == mSaveButton)
      SaveKits();
   if (button == mNewKitButton)
      CreateKit();
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
   mModuleSaveData.LoadInt("drumkit", moduleInfo, 5, 0, 100, true);
   mModuleSaveData.LoadString("twister", moduleInfo, "", FillDropdown<MidiController*>);
   
   SetUpFromSaveData();
}

void DrumSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   LoadKit(mModuleSaveData.GetInt("drumkit"));
   
   mTwister = TheSynth->FindMidiController(mModuleSaveData.GetString("twister"));
   if (mTwister)
      mTwister->AddListener(this, 0);
}

DrumSynth::DrumSynthHit::DrumSynthHit(DrumSynth* parent, int index, int x, int y)
: mPhase(0)
, mVolSlider(nullptr)
, mFreqSlider(nullptr)
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
   mFreqSlider = new FloatSlider(mParent, ("freq"+ofToString(mIndex)).c_str(), mX+35, mY+120,100,15,&mData.mFreq,25,1600);
   mToneType = new RadioButton(mParent, ("type"+ofToString(mIndex)).c_str(), mX+5, mY+75,(int*)(&mData.mTone.mOsc.mType));
   mToneAdsrDisplay = new ADSRDisplay(mParent, ("adsrtone"+ofToString(mIndex)).c_str(), mX+5, mY+15, 60,40,mData.mTone.GetADSR());
   mFreqAdsrDisplay = new ADSRDisplay(mParent, ("adsrfreq"+ofToString(mIndex)).c_str(), mX+35, mY+76,100,44,&mData.mFreqAdsr);
   mVolNoiseSlider = new FloatSlider(mParent, ("noise"+ofToString(mIndex)).c_str(), mX+70, mY+55,60,15,&mData.mVolNoise,0,1,2);
   mNoiseAdsrDisplay = new ADSRDisplay(mParent, ("adsrnoise"+ofToString(mIndex)).c_str(), mX+70, mY+15,60,40,mData.mNoise.GetADSR());
   
   mToneType->AddLabel("sin",kOsc_Sin);
   mToneType->AddLabel("saw",kOsc_Saw);
   mToneType->AddLabel("squ",kOsc_Square);
   mToneType->AddLabel("tri",kOsc_Tri);
   
   mFreqAdsrDisplay->SetMaxTime(500);
   mToneAdsrDisplay->SetMaxTime(500);
   mNoiseAdsrDisplay->SetMaxTime(500);
   
   mFreqSlider->SetMode(FloatSlider::kLogarithmic);
}

void DrumSynth::DrumSynthHit::Play(double time, float velocity)
{
   mData.mFreqAdsr.Start(time, 1);
   mData.mTone.Start(time, velocity);
   mData.mNoise.Start(time,velocity);
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
      float freq = mData.mFreqAdsr.Value(time) * mData.mFreq;
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
   ofRect(mX+4,mY+14,62,42);
   ofSetColor(0,255,0);
   ofRect(mX+69,mY+14,62,42);
   ofSetColor(0,0,255);
   ofRect(mX+34,mY+75,102,46);
   
   mToneAdsrDisplay->Draw();
   mVolSlider->Draw();
   mNoiseAdsrDisplay->Draw();
   mVolNoiseSlider->Draw();
   mToneType->Draw();
   mFreqAdsrDisplay->Draw();
   mFreqSlider->Draw();
   
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
, mFreq(150)
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




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
//  DrumSynth.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 8/5/14.
//
//

#include "DrumSynth.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "MidiController.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "IAudioReceiver.h"
#include "ADSRDisplay.h"

#define DRUMSYNTH_NO_CUTOFF 10000

namespace
{
   const int kPadYOffset = 20;
}

DrumSynth::DrumSynth()
{
   for (int i = 0; i < (int)mHits.size(); ++i)
   {
      int x = (i % DRUMSYNTH_PADS_HORIZONTAL) * DRUMSYNTH_PAD_WIDTH + 5;
      int y = (1 - (i / DRUMSYNTH_PADS_HORIZONTAL)) * DRUMSYNTH_PAD_HEIGHT + kPadYOffset;
      mHits[i] = new DrumSynthHit(this, i, x, y);
      if (i == 0)
         mHits[i]->mData.mVol = .5f;
   }
}

void DrumSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mVolSlider, "vol", &mVolume, 0, 2);
   ENDUIBLOCK0();

   for (size_t i = 0; i < mHits.size(); ++i)
      mHits[i]->CreateUIControls();
}

DrumSynth::~DrumSynth()
{
   for (size_t i = 0; i < mHits.size(); ++i)
      delete mHits[i];
}

void DrumSynth::Process(double time)
{
   PROFILER(DrumSynth);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || (target == nullptr && !mUseIndividualOuts))
      return;

   int numChannels = mMonoOutput ? 1 : 2;

   ComputeSliders(0);
   SyncOutputBuffer(numChannels);

   int oversampling = mOversampling;
   int bufferSize = gBufferSize;
   double sampleIncrementMs = gInvSampleRateMs;
   double sampleRate = gSampleRate;

   if (oversampling != 1)
   {
      bufferSize *= oversampling;
      sampleIncrementMs /= oversampling;
      sampleRate *= oversampling;
   }

   float volSq = mVolume * mVolume;

   if (mUseIndividualOuts)
   {
      for (int i = 0; i < (int)mHits.size(); ++i)
      {
         int hitOversampling = oversampling;
         int hitBufferSize = bufferSize;

         if (GetTarget(i + 1) != nullptr)
         {
            Clear(gWorkBuffer, hitBufferSize);

            mHits[i]->Process(time, gWorkBuffer, hitBufferSize, oversampling, sampleRate, sampleIncrementMs);

            //assume power-of-two
            while (hitOversampling > 1)
            {
               for (int j = 0; j < hitBufferSize; ++j)
                  gWorkBuffer[j] = (gWorkBuffer[j * 2] + gWorkBuffer[j * 2 + 1]) / 2;
               hitOversampling /= 2;
               hitBufferSize /= 2;
            }

            Mult(gWorkBuffer, volSq, hitBufferSize);
            auto* targetBuffer = GetTarget(i + 1)->GetBuffer();
            mHits[i]->mIndividualOutput->mVizBuffer->SetNumChannels(numChannels);
            for (int ch = 0; ch < numChannels; ++ch)
            {
               mHits[i]->mIndividualOutput->mVizBuffer->WriteChunk(gWorkBuffer, hitBufferSize, ch);
               Add(targetBuffer->GetChannel(ch), gWorkBuffer, hitBufferSize);
            }
         }
      }
   }
   else
   {
      Clear(gWorkBuffer, bufferSize);

      for (size_t i = 0; i < mHits.size(); ++i)
         mHits[i]->Process(time, gWorkBuffer, bufferSize, oversampling, sampleRate, sampleIncrementMs);

      //assume power-of-two
      while (oversampling > 1)
      {
         for (int i = 0; i < bufferSize; ++i)
            gWorkBuffer[i] = (gWorkBuffer[i * 2] + gWorkBuffer[i * 2 + 1]) / 2;
         oversampling /= 2;
         bufferSize /= 2;
      }

      Mult(gWorkBuffer, volSq, bufferSize);

      for (int ch = 0; ch < numChannels; ++ch)
      {
         GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, ch);
         Add(target->GetBuffer()->GetChannel(ch), gWorkBuffer, bufferSize);
      }
   }
}

void DrumSynth::PlayNote(NoteMessage note)
{
   if (note.pitch >= 0 && note.pitch < mHits.size())
   {
      if (note.velocity > 0)
         mHits[note.pitch]->Play(note.time, note.velocity / 127.0f);
   }
}

void DrumSynth::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   x -= 5;
   y -= kPadYOffset;
   if (x < 0 || y < 0)
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

   ofPushMatrix();
   for (size_t i = 0; i < mHits.size(); ++i)
   {
      ofPushStyle();
      if (mHits[i]->Level() > 0)
      {
         ofFill();
         ofSetColor(200, 100, 0, gModuleDrawAlpha * sqrtf(mHits[i]->Level()));
         ofRect(mHits[i]->mX, mHits[i]->mY, DRUMSYNTH_PAD_WIDTH, DRUMSYNTH_PAD_HEIGHT);
      }
      ofSetColor(200, 100, 0, gModuleDrawAlpha);
      ofNoFill();
      ofRect(mHits[i]->mX, mHits[i]->mY, DRUMSYNTH_PAD_WIDTH, DRUMSYNTH_PAD_HEIGHT);
      ofPopStyle();

      ofSetColor(255, 255, 255, gModuleDrawAlpha);

      std::string name = ofToString(i);
      DrawTextNormal(name, mHits[i]->mX + 5, mHits[i]->mY + 12);

      mHits[i]->Draw();
   }
   ofPopMatrix();
}

int DrumSynth::GetAssociatedSampleIndex(int x, int y)
{
   int pos = x + (DRUMSYNTH_PADS_VERTICAL - 1 - y) * DRUMSYNTH_PADS_HORIZONTAL;
   if (pos < DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL)
      return pos;
   return -1;
}

void DrumSynth::GetModuleDimensions(float& width, float& height)
{
   width = 10 + MIN(mHits.size(), DRUMSYNTH_PADS_HORIZONTAL) * DRUMSYNTH_PAD_WIDTH;
   height = 2 + kPadYOffset + mHits.size() / DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PAD_HEIGHT;
}

void DrumSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void DrumSynth::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void DrumSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void DrumSynth::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void DrumSynth::ButtonClicked(ClickButton* button, double time)
{
}

void DrumSynth::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
}

void DrumSynth::TextEntryComplete(TextEntry* entry)
{
}

void DrumSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("individual_outs", moduleInfo, false);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);
   EnumMap oversamplingMap;
   oversamplingMap["1"] = 1;
   oversamplingMap["2"] = 2;
   oversamplingMap["4"] = 4;
   oversamplingMap["8"] = 8;
   mModuleSaveData.LoadEnum<int>("oversampling", moduleInfo, 1, nullptr, &oversamplingMap);

   SetUpFromSaveData();
}

void DrumSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   bool useIndividualOuts = mModuleSaveData.GetBool("individual_outs");
   if (useIndividualOuts)
   {
      for (size_t i = 0; i < mHits.size(); ++i)
      {
         if (mHits[i]->mIndividualOutput == nullptr)
            mHits[i]->mIndividualOutput = new IndividualOutput(mHits[i]);
      }
   }
   GetPatchCableSource()->SetShowing(!useIndividualOuts);

   mUseIndividualOuts = useIndividualOuts;

   mMonoOutput = mModuleSaveData.GetBool("mono");

   mOversampling = mModuleSaveData.GetEnum<int>("oversampling");
}

DrumSynth::DrumSynthHit::DrumSynthHit(DrumSynth* parent, int index, int x, int y)
: mParent(parent)
, mIndex(index)
, mX(x)
, mY(y)
{
   mFilter.SetFilterType(kFilterType_Lowpass);
   mFilter.SetFilterParams(1000, sqrt(2) / 2);
}

DrumSynth::DrumSynthHit::~DrumSynthHit()
{
   delete mIndividualOutput;
}

void DrumSynth::DrumSynthHit::CreateUIControls()
{
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER mParent //change owner

   float width, height;
   float kColumnWidth = (DRUMSYNTH_PAD_WIDTH - 5 * 2 - 3) * .5f;
   UIBLOCK(mX + 5, mY + 15, kColumnWidth);

   UICONTROL_CUSTOM(mToneAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrtone" + ofToString(mIndex)).c_str()), kColumnWidth, 36, mData.mTone.GetADSR()));
   FLOATSLIDER(mVolSlider, ("vol" + ofToString(mIndex)).c_str(), &mData.mVol, 0, 1);

   UIBLOCK_NEWCOLUMN();
   UICONTROL_CUSTOM(mNoiseAdsrDisplay, new ADSRDisplay(UICONTROL_BASICS(("adsrnoise" + ofToString(mIndex)).c_str()), kColumnWidth, 36, mData.mNoise.GetADSR()));
   FLOATSLIDER_DIGITS(mVolNoiseSlider, ("noise" + ofToString(mIndex)).c_str(), &mData.mVolNoise, 0, 1, 2);
   ENDUIBLOCK(width, height);

   UIBLOCK(mX + 5, height + 3);
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
   FLOATSLIDER_DIGITS(mFilterQSlider, ("q" + ofToString(mIndex)).c_str(), &mData.mQ, .1, 20, 3);

   ENDUIBLOCK0();

#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset

   mToneType->AddLabel("sin", kOsc_Sin);
   mToneType->AddLabel("saw", kOsc_Saw);
   mToneType->AddLabel("squ", kOsc_Square);
   mToneType->AddLabel("tri", kOsc_Tri);

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
   mData.mNoise.GetADSR()->Start(time, velocity, envelopeScale);
}

void DrumSynth::DrumSynthHit::Process(double time, float* out, int bufferSize, int oversampling, double sampleRate, double sampleIncrementMs)
{
   if (mData.mTone.GetADSR()->IsDone(time) && mData.mNoise.GetADSR()->IsDone(time))
   {
      mLevel.Reset();
      mPhase = 0;
      return;
   }

   for (size_t i = 0; i < bufferSize; ++i)
   {
      float freq = ofLerp(mData.mFreqMin, mData.mFreqMax, mData.mFreqAdsr.Value(time));
      if (mData.mCutoffMax != DRUMSYNTH_NO_CUTOFF)
      {
         mFilter.SetSampleRate(sampleRate);
         mFilter.SetFilterParams(ofLerp(mData.mCutoffMin, mData.mCutoffMax, mData.mFilterAdsr.Value(time)), mData.mQ);
      }
      float phaseInc = GetPhaseInc(freq) / oversampling;

      float sample = mData.mTone.Audio(time, mPhase) * mData.mVol * mData.mVol;
      float noise = mData.mNoise.Audio(time, mPhase);
      noise *= noise * (noise > 0 ? 1 : -1); //square but keep sign
      sample += noise * mData.mVolNoise * mData.mVolNoise;
      if (mData.mCutoffMax != DRUMSYNTH_NO_CUTOFF)
         sample = mFilter.Filter(sample);
      mLevel.Process(&sample, 1);
      out[i] += sample;

      mPhase += phaseInc;
      while (mPhase > FTWO_PI)
      {
         mPhase -= FTWO_PI;
      }

      time += sampleIncrementMs;
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
   mFreqAdsr.GetStageData(0).time = 1;
   mFreqAdsr.GetStageData(0).target = 1;
   mFreqAdsr.GetStageData(1).time = 500;
   mFreqAdsr.GetStageData(1).target = 0;

   mFilterAdsr.SetNumStages(2);
   mFilterAdsr.GetHasSustainStage() = false;
   mFilterAdsr.GetStageData(0).time = 1;
   mFilterAdsr.GetStageData(0).target = 1;
   mFilterAdsr.GetStageData(1).time = 500;
   mFilterAdsr.GetStageData(1).target = 0;
}

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
#include "UIControlMacros.h"

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
, mUnisonSlider(nullptr)
, mUnisonWidthSlider(nullptr)
, mShuffleSlider(nullptr)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mLengthMultiplier(1)
, mLengthMultiplierSlider(nullptr)
, mWriteBuffer(gBufferSize)
, mDrawOsc(kOsc_Square)
{
   mVoiceParams.mAdsr.Set(10,0,1,10);
   mVoiceParams.mVol = .25f;
   mVoiceParams.mPulseWidth = .5f;
   mVoiceParams.mSync = false;
   mVoiceParams.mSyncFreq = 200;
   mVoiceParams.mMult = 1;
   mVoiceParams.mOscType = kOsc_Square;
   mVoiceParams.mDetune = 0;
   mVoiceParams.mFilterAdsr.Set(1,0,1,1000);
   mVoiceParams.mFilterCutoffMax = SINGLEOSCILLATOR_NO_CUTOFF;
   mVoiceParams.mFilterCutoffMin = 10;
   mVoiceParams.mFilterQ = sqrt(2)/2;
   mVoiceParams.mShuffle = 0;
   mVoiceParams.mPhaseOffset = 0;
   mVoiceParams.mUnison = 1;
   mVoiceParams.mUnisonWidth = 0;
   mVoiceParams.mVelToVolume = .5f;
   mVoiceParams.mVelToEnvelope = 0;
   mVoiceParams.mSoften = 0;
   mVoiceParams.mLiteCPUMode = false;
   
   mPolyMgr.Init(kVoiceType_SingleOscillator, &mVoiceParams);
}

namespace
{
   const float kColumnWidth = 90;
   const float kGap = 5;
}

void SingleOscillator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   float width, height;
   UIBLOCK(3, 42, kColumnWidth);
   DROPDOWN(mOscSelector, "osc", (int*)(&mVoiceParams.mOscType), kColumnWidth / 2); UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mMultSelector, "mult", &mMult, kColumnWidth / 2-3); UIBLOCK_NEWLINE();
   FLOATSLIDER_DIGITS(mPulseWidthSlider,"pw",&mVoiceParams.mPulseWidth,0.01f,.99f,2);
   FLOATSLIDER(mShuffleSlider,"shuffle", &mVoiceParams.mShuffle, 0, 1);
   FLOATSLIDER(mSoftenSlider, "soften", &mVoiceParams.mSoften, 0, 1);
   FLOATSLIDER(mPhaseOffsetSlider,"phase",&mVoiceParams.mPhaseOffset,0,TWO_PI);
   CHECKBOX(mSyncCheckbox, "sync", &mVoiceParams.mSync); UIBLOCK_SHIFTRIGHT();
   UIBLOCK_PUSHSLIDERWIDTH(47);
   FLOATSLIDER(mSyncFreqSlider, "syncf", &mVoiceParams.mSyncFreq, 10, 999.9f); UIBLOCK_NEWLINE();
   UIBLOCK_POPSLIDERWIDTH();
   ENDUIBLOCK(mWidth, mHeight);
   
   UIBLOCK(3 + kGap + kColumnWidth, 3, kColumnWidth);
   UICONTROL_CUSTOM(mADSRDisplay, new ADSRDisplay(UICONTROL_BASICS("env"), kColumnWidth,36,&mVoiceParams.mAdsr));
   FLOATSLIDER(mVolSlider, "vol", &mVoiceParams.mVol, 0, 1);
   FLOATSLIDER_DIGITS(mDetuneSlider, "detune", &mVoiceParams.mDetune, -.05f, .05f, 3);
   INTSLIDER(mUnisonSlider, "unison", &mVoiceParams.mUnison, 1, SingleOscillatorVoice::kMaxUnison);
   FLOATSLIDER(mUnisonWidthSlider, "width", &mVoiceParams.mUnisonWidth, 0, 1);
   FLOATSLIDER_DIGITS(mLengthMultiplierSlider, "adsr len", &mLengthMultiplier, .01f, 10, 1);
   CHECKBOX(mLiteCPUModeCheckbox, "lite cpu", &mVoiceParams.mLiteCPUMode);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   UIBLOCK(3 + (kGap + kColumnWidth) * 2, 3, kColumnWidth);
   UICONTROL_CUSTOM(mFilterADSRDisplay, new ADSRDisplay(UICONTROL_BASICS("envfilter"), kColumnWidth, 36, &mVoiceParams.mFilterAdsr));
   FLOATSLIDER(mFilterCutoffMaxSlider, "fmax", &mVoiceParams.mFilterCutoffMax, 10, SINGLEOSCILLATOR_NO_CUTOFF);
   FLOATSLIDER(mFilterCutoffMinSlider, "fmin", &mVoiceParams.mFilterCutoffMin, 10, SINGLEOSCILLATOR_NO_CUTOFF);
   FLOATSLIDER(mFilterQSlider, "q", &mVoiceParams.mFilterQ, .1, 20);
   FLOATSLIDER(mVelToVolumeSlider, "vel2vol", &mVoiceParams.mVelToVolume, 0, 1);
   FLOATSLIDER(mVelToEnvelopeSlider, "vel2env", &mVoiceParams.mVelToEnvelope, -1, 1);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   mOscSelector->AddLabel("sin", kOsc_Sin);
   mOscSelector->AddLabel("squ", kOsc_Square);
   mOscSelector->AddLabel("tri", kOsc_Tri);
   mOscSelector->AddLabel("saw", kOsc_Saw);
   mOscSelector->AddLabel("-saw", kOsc_NegSaw);
   mOscSelector->AddLabel("noise", kOsc_Random);

   mSyncFreqSlider->SetShowName(false);
   
   mFilterCutoffMaxSlider->SetMaxValueDisplay("none");

   mLengthMultiplierSlider->SetMode(FloatSlider::kSquare);
   
   mMultSelector->AddLabel("1/8", -8);
   mMultSelector->AddLabel("1/7", -7);
   mMultSelector->AddLabel("1/6", -6);
   mMultSelector->AddLabel("1/5", -5);
   mMultSelector->AddLabel("1/4", -4);
   mMultSelector->AddLabel("1/3", -3);
   mMultSelector->AddLabel("1/2", -2);
   mMultSelector->AddLabel("1", 1);
   mMultSelector->AddLabel("1.5", -1);
   mMultSelector->AddLabel("2", 2);
   mMultSelector->AddLabel("3", 3);
   mMultSelector->AddLabel("4", 4);
   mMultSelector->AddLabel("5", 5);
   mMultSelector->AddLabel("6", 6);
   mMultSelector->AddLabel("7", 7);
   mMultSelector->AddLabel("8", 8);

   mFilterCutoffMaxSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMinSlider->SetMode(FloatSlider::kSquare);
   mFilterQSlider->SetMode(FloatSlider::kSquare);
}

SingleOscillator::~SingleOscillator()
{
}

void SingleOscillator::Process(double time)
{
   PROFILER(SingleOscillator);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;
   
   mNoteInputBuffer.Process(time);
   
   ComputeSliders(0);
   
   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   
   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void SingleOscillator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
      float adsrScale = SingleOscillatorVoice::GetADSRScale(velocity/127.0f, mVoiceParams.mVelToEnvelope);
      mVoiceParams.mAdsr.Start(time, 1, adsrScale);         //for visualization
      mVoiceParams.mFilterAdsr.Start(time, 1, adsrScale);   //for visualization
   }
   else
   {
      mPolyMgr.Stop(time, pitch);
      mVoiceParams.mAdsr.Stop(time, false);         //for visualization
      mVoiceParams.mFilterAdsr.Stop(time, false);   //for visualization
   }
   
   if (mDrawDebug)
   {
      std::vector<std::string> lines = ofSplitString(mDebugLines, "\n");
      mDebugLines = "";
      const int kNumDisplayLines = 10;
      for (int i=0; i<kNumDisplayLines-1; ++i)
      {
         int lineIndex = (int)lines.size()-(kNumDisplayLines-1) + i;
         if (lineIndex >= 0)
            mDebugLines += lines[lineIndex] + "\n";
      }
      std::string debugLine = "PlayNote(" + ofToString(time / 1000) + ", " + ofToString(pitch) + ", " + ofToString(velocity) + ", " + ofToString(voiceIdx) + ")";
      mDebugLines += debugLine;
      ofLog() << debugLine;
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
   mUnisonSlider->Draw();
   mUnisonWidthSlider->Draw();
   mShuffleSlider->Draw();
   mVelToVolumeSlider->Draw();
   mVelToEnvelopeSlider->Draw();
   mFilterADSRDisplay->Draw();
   mFilterCutoffMaxSlider->Draw();
   mFilterCutoffMinSlider->Draw();
   mFilterQSlider->Draw();
   if (mVoiceParams.mFilterCutoffMax == SINGLEOSCILLATOR_NO_CUTOFF)
   {
      ofPushStyle();
      ofSetColor(0, 0, 0, 100);
      ofFill();
      ofRect(mFilterADSRDisplay->GetRect(true).grow(1));
      ofRect(mFilterCutoffMinSlider->GetRect(true));
      ofRect(mFilterQSlider->GetRect(true));
      ofPopStyle();
   }
   mADSRDisplay->Draw();
   mMultSelector->Draw();
   mSoftenSlider->Draw();
   mLiteCPUModeCheckbox->Draw();
   
   {
      ofPushStyle();
      int x = 3;
      int y = 3;
      int height = 36;
      int width = kColumnWidth;
      
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
         if (mVoiceParams.mSync)
         {
            FloatWrap(phase, FTWO_PI);
            phase *= mVoiceParams.mSyncFreq / 200;
         }
         if (mDrawOsc.GetShuffle() > 0)
            phase *= 2;
         mDrawOsc.SetSoften(mVoiceParams.mSoften);
         float value = mDrawOsc.Value(phase);
         ofVertex(i + x, ofMap(value,-1,1,0,height) + y);
      }
      ofEndShape(false);
      ofPopStyle();
   }

   DrawTextRightJustify("wave", kGap + kColumnWidth - 3, 15);
   DrawTextRightJustify("volume", (kGap + kColumnWidth) * 2 - 3, 15);
   ofPushStyle();
   if (mVoiceParams.mFilterCutoffMax == SINGLEOSCILLATOR_NO_CUTOFF)
      ofSetColor(100, 100, 100);
   DrawTextRightJustify("filter", (kGap + kColumnWidth) * 3 - 3, 15);
   ofPopStyle();
}
 
void SingleOscillator::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(mWidth+3, 0);
      DrawTextNormal(mDebugLines, 0, mHeight+15);
   }
}

void SingleOscillator::UpdateOldControlName(std::string& oldName)
{
   IDrawableModule::UpdateOldControlName(oldName);

   if (oldName == "f")
      oldName = "fmax";
}

void SingleOscillator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("vol", moduleInfo, .5, mVolSlider);
   mModuleSaveData.LoadEnum<OscillatorType>("osc", moduleInfo, kOsc_Sin, mOscSelector);
   mModuleSaveData.LoadFloat("detune", moduleInfo, 0, mDetuneSlider);
   mModuleSaveData.LoadBool("pressure_envelope", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);

   SetUpFromSaveData();
}

void SingleOscillator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);
   
   bool mono = mModuleSaveData.GetBool("mono");
   mWriteBuffer.SetNumActiveChannels(mono ? 1 : 2);
}


void SingleOscillator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mMultSelector)
   {
      if (mMult > 0)
         mVoiceParams.mMult = mMult;
      else if (mMult == -1)   //-1 is special case for 1.5
         mVoiceParams.mMult = 1.5f;
      else                    //other negative numbers mean 1/-x
         mVoiceParams.mMult = -1.0f/mMult;
   }
   if (list == mOscSelector)
      mDrawOsc.SetType(mVoiceParams.mOscType);
}

void SingleOscillator::RadioButtonUpdated(RadioButton* list, int oldVal)
{
}

void SingleOscillator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
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
   if (checkbox == mEnabledCheckbox)
      mPolyMgr.KillAll();
}


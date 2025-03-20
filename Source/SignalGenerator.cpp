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
#include "UIControlMacros.h"

SignalGenerator::SignalGenerator()
{
   mWriteBuffer = new float[gBufferSize];

   mOsc.Start(0, 1);
}

void SignalGenerator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UIBLOCK_PUSHSLIDERWIDTH(171);
   FLOATSLIDER(mFreqSlider, "freq", &mFreq, 1, 4000);
   DROPDOWN(mFreqModeSelector, "freq mode", (int*)(&mFreqMode), 55);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mOscSelector, "osc", (int*)(&mOscType), 50);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_PUSHSLIDERWIDTH(60);
   FLOATSLIDER(mPulseWidthSlider, "pw", &mPulseWidth, 0.01, .99);
   UIBLOCK_NEWLINE();
   UIBLOCK_PUSHSLIDERWIDTH(80);
   FLOATSLIDER(mFreqSliderAmountSlider, "slider", &mFreqSliderAmount, 0, 1);
   UIBLOCK_SHIFTLEFT();
   FLOATSLIDER(mFreqRampTimeSlider, "ramp", &mFreqRampTime, 0, 1000);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(8);
   FLOATSLIDER(mShuffleSlider, "shuffle", &mShuffle, 0, 1);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mSoftenSlider, "soften", &mSoften, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(8);
   FLOATSLIDER_DIGITS(mPhaseOffsetSlider, "phase", &mPhaseOffset, 0, 1, 3);
   UIBLOCK_NEWLINE();
   DROPDOWN(mMultSelector, "mult", &mMult, 40);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(5);
   DROPDOWN(mSyncModeSelector, "syncmode", (int*)(&mSyncMode), 60);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_PUSHSLIDERWIDTH(60);
   FLOATSLIDER(mSyncFreqSlider, "syncf", &mSyncFreq, 10, 999.9);
   UIBLOCK_SHIFTLEFT();
   FLOATSLIDER(mSyncRatioSlider, "syncratio", &mSyncRatio, .1, 10.0);
   UIBLOCK_NEWLINE();
   UIBLOCK_PUSHSLIDERWIDTH(80);
   FLOATSLIDER(mVolSlider, "vol", &mVol, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   UIBLOCK_SHIFTX(8);
   FLOATSLIDER_DIGITS(mDetuneSlider, "detune", &mDetune, -.05, .05, 3);
   ENDUIBLOCK0();

   mSyncModeSelector->AddLabel("no sync", (int)Oscillator::SyncMode::None);
   mSyncModeSelector->AddLabel("ratio", (int)Oscillator::SyncMode::Ratio);
   mSyncModeSelector->AddLabel("freq", (int)Oscillator::SyncMode::Frequency);

   mSyncFreqSlider->SetShowName(false);
   mSyncRatioSlider->SetShowName(false);
   mSyncRatioSlider->SetMode(FloatSlider::kSquare);
   mSyncFreqSlider->PositionTo(mSyncModeSelector, kAnchor_Right);
   mSyncRatioSlider->PositionTo(mSyncModeSelector, kAnchor_Right);

   SetFreqMode(kFreqMode_Instant);

   mOscSelector->AddLabel("sin", kOsc_Sin);
   mOscSelector->AddLabel("squ", kOsc_Square);
   mOscSelector->AddLabel("tri", kOsc_Tri);
   mOscSelector->AddLabel("saw", kOsc_Saw);
   mOscSelector->AddLabel("-saw", kOsc_NegSaw);
   mOscSelector->AddLabel("noise", kOsc_Random);

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

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   auto bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   Clear(mWriteBuffer, gBufferSize);
   for (int pos = 0; pos < bufferSize; ++pos)
   {
      ComputeSliders(pos);

      if (mResetPhaseAtMs > 0 && time > mResetPhaseAtMs)
      {
         mPhase = mPhaseOffset;
         mResetPhaseAtMs = -9999;
      }

      double volSq = mVol * mVol;

      if (mFreqMode == kFreqMode_Root)
         mFreq = TheScale->PitchToFreq(TheScale->ScaleRoot() + 24);
      else if (mFreqMode == kFreqMode_Ramp)
         mFreq = mFreqRamp.Value(time);
      else if (mFreqMode == kFreqMode_Slider)
         mFreq = ofLerp(mFreqSliderStart, mFreqSliderEnd, mFreqSliderAmount);

      double mult = mMult;
      if (mult < 0)
         mult = -1.0 / mult;
      double outputFreq = mFreq * exp2(mDetune) * mult;
      double phaseInc = GetPhaseInc(outputFreq);

      double syncPhaseInc = 0;
      if (mSyncMode == Oscillator::SyncMode::Frequency)
         syncPhaseInc = GetPhaseInc(mSyncFreq);
      else if (mSyncMode == Oscillator::SyncMode::Ratio)
         syncPhaseInc = GetPhaseInc(outputFreq * mSyncRatio);

      mPhase += phaseInc;
      if (ofAlmostEquel(mPhase, std::numeric_limits<double>::infinity()))
      {
         ofLog() << "Infinite phase.";
      }
      else
      {
         while (mPhase > TWO_PI * 2)
         {
            mPhase -= TWO_PI * 2;
            mSyncPhase = 0;
         }
      }
      mSyncPhase += syncPhaseInc;

      if (mSyncMode != Oscillator::SyncMode::None)
         mWriteBuffer[pos] += mOsc.Audio(time, mSyncPhase) * volSq;
      else
         mWriteBuffer[pos] += mOsc.Audio(time, mPhase + mPhaseOffset * TWO_PI) * volSq;

      time += gInvSampleRateMs;
   }
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);

   Add(out, mWriteBuffer, bufferSize);
}

void SignalGenerator::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      if (mFreqMode == kFreqMode_Instant)
      {
         mFreq = TheScale->PitchToFreq(note.pitch);
      }
      else if (mFreqMode == kFreqMode_Ramp)
      {
         mFreqRamp.Start(note.time, TheScale->PitchToFreq(note.pitch), note.time + mFreqRampTime);
      }
      else if (mFreqMode == kFreqMode_Slider)
      {
         double freq = TheScale->PitchToFreq(note.pitch);
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

void SignalGenerator::OnPulse(double time, double velocity, int flags)
{
   mResetPhaseAtMs = time;
}

void SignalGenerator::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void SignalGenerator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSyncFreqSlider->SetShowing(mSyncMode == Oscillator::SyncMode::Frequency);
   mSyncRatioSlider->SetShowing(mSyncMode == Oscillator::SyncMode::Ratio);

   mVolSlider->Draw();
   mPulseWidthSlider->Draw();
   mShuffleSlider->Draw();
   mMultSelector->Draw();
   mSyncModeSelector->Draw();
   mSyncFreqSlider->Draw();
   mSyncRatioSlider->Draw();
   mOscSelector->Draw();
   mDetuneSlider->Draw();
   mFreqSlider->Draw();
   mFreqModeSelector->Draw();
   mFreqSliderAmountSlider->Draw();
   mFreqRampTimeSlider->Draw();
   mSoftenSlider->Draw();
   mPhaseOffsetSlider->Draw();
}

void SignalGenerator::GetModuleDimensions(double& width, double& height)
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
   mModuleSaveData.LoadFloat("detune", moduleInfo, 0, mDetuneSlider);
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

void SignalGenerator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mOscSelector)
   {
      mOsc.SetType(mOscType);
      mSoftenSlider->SetShowing(mOscType == kOsc_Square || mOscType == kOsc_Saw || mOscType == kOsc_NegSaw);
   }
   if (list == mFreqModeSelector)
      SetFreqMode(mFreqMode);
}

void SignalGenerator::FloatSliderUpdated(FloatSlider* slider, double oldVal, double time)
{
   if (slider == mPulseWidthSlider)
      mOsc.SetPulseWidth(mPulseWidth);
   if (slider == mShuffleSlider)
      mOsc.mOsc.SetShuffle(mShuffle);
   if (slider == mFreqSlider)
   {
      if (mFreqMode == kFreqMode_Ramp)
         mFreqRamp.Start(time, mFreq, time + mFreqRampTime);
   }
   if (slider == mSoftenSlider)
      mOsc.mOsc.SetSoften(mSoften);
}

void SignalGenerator::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SignalGenerator::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void SignalGenerator::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void SignalGenerator::LoadState(FileStreamIn& in, int rev)
{
   mLoadRev = rev;

   IDrawableModule::LoadState(in, rev);
}

bool SignalGenerator::LoadOldControl(FileStreamIn& in, std::string& oldName)
{
   if (mLoadRev < 1)
   {
      if (oldName == "sync")
      {
         //load checkbox
         int checkboxRev;
         in >> checkboxRev;
         float checkboxVal;
         in >> checkboxVal;
         if (checkboxVal > 0)
            mSyncMode = Oscillator::SyncMode::Frequency;
         else
            mSyncMode = Oscillator::SyncMode::None;
         return true;
      }
   }
   return false;
}

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
//  Tracker.cpp
//  Bespoke
//

#include "Tracker.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "IAudioReceiver.h"
#include "Profiler.h"
#include "ofxJSONElement.h"
#include "SearchPanel.h"
#include "juce_core/juce_core.h"

namespace
{
   const float kNumX = 6;
   const float kSampleX = 24;
   const float kSampleW = 118;
   const float kSliderW = 80;
   const float kRepeatW = 64;
   const float kColGap = 6;
   const float kClearX = kSampleX + kSampleW + 4; //per-step delete button
   const float kClearW = 15;
   const float kNextX = kClearX + kClearW + 3; //per-step next-sample button
   const float kNextW = 16;
   const float kVolX = kNextX + kNextW + 8;
   const float kDecayX = kVolX + kSliderW + kColGap;
   const float kPitchX = kDecayX + kSliderW + kColGap;
   const float kRepeatX = kPitchX + kSliderW + kColGap;
   const float kRandX = kRepeatX + kRepeatW + kColGap;
   const float kRandW = 22;
   const float kRowsTop = 42;
   const float kRowH = 18;
}

Tracker::Tracker()
: mWriteBuffer(gBufferSize)
{
   mWriteBuffer.SetNumActiveChannels(2);
}

Tracker::~Tracker()
{
   TheTransport->RemoveListener(this);
}

void Tracker::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mIntervalSelector = new DropdownList(this, "rate", 34, 2, (int*)(&mInterval), 46);
   mNumStepsSlider = new IntSlider(this, "steps", 150, 2, 120, 15, &mNumSteps, 1, kMaxSteps);
   mRandomizeButton = new ClickButton(this, "randomize", 290, 2);
   mGlobalVolumeSlider = new FloatSlider(this, "volume", 400, 2, 110, 15, &mGlobalVolume, 0.0f, 2.0f);

   //full set of transport rates, fastest bar-divisions through slow note values
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   for (int i = 0; i < kMaxSteps; ++i)
   {
      float y = kRowsTop + i * kRowH;
      mSteps[i].mVolSlider = new FloatSlider(this, ("vol" + ofToString(i)).c_str(), (int)kVolX, (int)y, (int)kSliderW, 15, &mSteps[i].mVol, 0, 1);
      mSteps[i].mDecaySlider = new FloatSlider(this, ("dec" + ofToString(i)).c_str(), (int)kDecayX, (int)y, (int)kSliderW, 15, &mSteps[i].mDecayMs, 5, 2000);
      mSteps[i].mPitchSlider = new FloatSlider(this, ("pitch" + ofToString(i)).c_str(), (int)kPitchX, (int)y, (int)kSliderW, 15, &mSteps[i].mPitch, -24, 24);
      mSteps[i].mRepeatSlider = new IntSlider(this, ("rep" + ofToString(i)).c_str(), (int)kRepeatX, (int)y, (int)kRepeatW, 15, &mSteps[i].mRepeat, 1, 16);
      mSteps[i].mDecaySlider->SetMode(FloatSlider::kSquare);
      mSteps[i].mClearButton = new ClickButton(this, ("clr" + ofToString(i)).c_str(), (int)kClearX, (int)y);
      mSteps[i].mClearButton->SetLabel("x"); //delete/reset the loaded sample
      mSteps[i].mNextButton = new ClickButton(this, ("nxt" + ofToString(i)).c_str(), (int)kNextX, (int)y);
      mSteps[i].mNextButton->SetLabel(">"); //switch to the next sample in the scanned library
      mSteps[i].mRandButton = new ClickButton(this, ("rnd" + ofToString(i)).c_str(), (int)kRandX, (int)y);
      mSteps[i].mRandButton->SetLabel("R"); //compact per-row randomize
   }

   UpdateDimensions();
}

void Tracker::Init()
{
   //Transport::AddListener asserts the module is initialized, so add the listener in Init() (after
   //IDrawableModule::Init sets that flag), not in CreateUIControls
   IDrawableModule::Init();
   UpdateTransportListener();
}

void Tracker::UpdateTransportListener()
{
   TransportListenerInfo* info = TheTransport->GetListenerInfo(this);
   if (info != nullptr)
      info->mInterval = mInterval;
   else
      TheTransport->AddListener(this, mInterval, OffsetInfo(0, false), false);
}

void Tracker::UpdateDimensions()
{
   mWidth = kRandX + kRandW + 6;
   mHeight = kRowsTop + mNumSteps * kRowH + 6;
}

void Tracker::OnTimeEvent(double time)
{
   if (!mEnabled || mNumSteps <= 0)
      return;

   mCurStep = (mCurStep + 1) % mNumSteps;

   StepData& s = mSteps[mCurStep];
   if (!s.mHasSample)
      return;

   //schedule the step's hits: 'repeat' evenly-spaced retriggers across the step's duration (a roll)
   double stepDur = TheTransport->GetDuration(mInterval);
   int rep = MAX(1, s.mRepeat);
   double hitInterval = stepDur / rep;
   for (int k = 0; k < rep; ++k)
   {
      ScheduledHit hit;
      hit.mStep = mCurStep;
      hit.mTime = time + k * hitInterval;
      mScheduledHits.push_back(hit);
   }

   //safety cap so a runaway can't grow the list unbounded
   if ((int)mScheduledHits.size() > 256)
      mScheduledHits.erase(mScheduledHits.begin(), mScheduledHits.begin() + (mScheduledHits.size() - 256));
}

void Tracker::TriggerStepHit(int step, double time)
{
   if (step < 0 || step >= kMaxSteps)
      return;
   StepData& s = mSteps[step];
   if (!s.mHasSample)
      return;

   //find a free voice; if all busy, steal voice 0
   for (auto& v : s.mVoices)
   {
      if (!v.mActive)
      {
         v.mActive = true;
         v.mOffset = 0;
         v.mEnvTimeMs = 0;
         return;
      }
   }
   s.mVoices[0].mActive = true;
   s.mVoices[0].mOffset = 0;
   s.mVoices[0].mEnvTimeMs = 0;
}

void Tracker::Process(double time)
{
   PROFILER(Tracker);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   //trigger any scheduled hits that are due at/before this block (block-level timing is fine here)
   for (size_t i = 0; i < mScheduledHits.size();)
   {
      if (mScheduledHits[i].mTime <= time)
      {
         TriggerStepHit(mScheduledHits[i].mStep, time);
         mScheduledHits.erase(mScheduledHits.begin() + i);
      }
      else
      {
         ++i;
      }
   }

   for (int s = 0; s < mNumSteps && s < kMaxSteps; ++s)
   {
      StepData& step = mSteps[s];
      if (!step.mHasSample)
         continue;
      ChannelBuffer* data = step.mSample.Data();
      int len = step.mSample.LengthInSamples();
      if (len <= 0 || data == nullptr)
         continue;

      float rate = powf(2.0f, step.mPitch / 12.0f) * step.mSample.GetSampleRateRatio();
      int dataCh = data->NumActiveChannels();

      for (auto& v : step.mVoices)
      {
         if (!v.mActive)
            continue;
         for (int i = 0; i < bufferSize; ++i)
         {
            if (v.mOffset >= len)
            {
               v.mActive = false;
               break;
            }
            float env = 1.0f - (float)(v.mEnvTimeMs / MAX(1.0f, step.mDecayMs)); //linear decay
            if (env <= 0)
            {
               v.mActive = false;
               break;
            }
            for (int ch = 0; ch < 2; ++ch)
            {
               int c = MIN(ch, dataCh - 1);
               float smp = GetInterpolatedSample(v.mOffset, data->GetChannel(c), len);
               mWriteBuffer.GetChannel(ch)[i] += smp * step.mVol * env * mGlobalVolume * 0.5f;
            }
            v.mOffset += rate;
            v.mEnvTimeMs += gInvSampleRateMs;
         }
      }
   }

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void Tracker::SampleDropped(int x, int y, Sample* sample)
{
   if (sample == nullptr || sample->LengthInSamples() <= 0)
      return;

   int row = (int)floorf((y - kRowsTop) / kRowH);
   if (row < 0 || row >= mNumSteps)
      return;

   mSteps[row].mSample.CopyFrom(sample);
   mSteps[row].mPath = sample->GetReadPath(); //remember where it came from, for the "next sample" button
   mSteps[row].mHasSample = true;
}

void Tracker::ClearStep(int step)
{
   if (step < 0 || step >= kMaxSteps)
      return;
   for (auto& v : mSteps[step].mVoices)
      v.mActive = false;
   mSteps[step].mHasSample = false;
   mSteps[step].mPath = "";
}

void Tracker::SwitchStepSample(int step, int dir)
{
   if (step < 0 || step >= kMaxSteps || TheSearchPanel == nullptr)
      return;
   std::string next;
   if (!TheSearchPanel->GetRelativeSamplePath(mSteps[step].mPath, dir, next) || next.empty())
      return;
   //briefly mark the step empty so the audio thread doesn't read the buffer while it's reloaded
   for (auto& v : mSteps[step].mVoices)
      v.mActive = false;
   mSteps[step].mHasSample = false;
   if (mSteps[step].mSample.Read(next.c_str()))
   {
      mSteps[step].mSample.SetName(juce::File(next).getFileName().toStdString());
      mSteps[step].mPath = next;
      mSteps[step].mHasSample = true;
   }
}

void Tracker::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("rate", 6, 14);
   mIntervalSelector->Draw();
   mNumStepsSlider->Draw();
   mRandomizeButton->Draw();
   mGlobalVolumeSlider->Draw();

   float hy = kRowsTop - 3;
   DrawTextNormal("sample", kSampleX, hy, 11);
   DrawTextNormal("del", kClearX - 2, hy, 9);
   DrawTextNormal("nxt", kNextX - 2, hy, 9);
   DrawTextNormal("vol", kVolX, hy, 11);
   DrawTextNormal("decay", kDecayX, hy, 11);
   DrawTextNormal("pitch", kPitchX, hy, 11);
   DrawTextNormal("rpt", kRepeatX, hy, 11);
   DrawTextNormal("rnd", kRandX, hy, 11);

   for (int i = 0; i < kMaxSteps; ++i)
   {
      bool shown = (i < mNumSteps);
      mSteps[i].mVolSlider->SetShowing(shown);
      mSteps[i].mDecaySlider->SetShowing(shown);
      mSteps[i].mPitchSlider->SetShowing(shown);
      mSteps[i].mRepeatSlider->SetShowing(shown);
      mSteps[i].mRandButton->SetShowing(shown);
      //delete + next-sample buttons only make sense when a sample is loaded
      mSteps[i].mClearButton->SetShowing(shown && mSteps[i].mHasSample);
      mSteps[i].mNextButton->SetShowing(shown);
      if (!shown)
         continue;

      float y = kRowsTop + i * kRowH;

      DrawTextNormal(ofToString(i + 1), (int)kNumX, (int)(y + 11), 12);

      //sample cell (drop target), highlighted when it's the currently-playing step
      ofPushStyle();
      if (i == mCurStep)
      {
         ofSetColor(90, 190, 90, 130);
         ofFill();
         ofRect(kSampleX, y, kSampleW, 15);
      }
      ofSetColor(190, 190, 190, 110);
      ofNoFill();
      ofRect(kSampleX, y, kSampleW, 15);
      ofPopStyle();

      std::string label = mSteps[i].mHasSample ? mSteps[i].mSample.Name() : "drop sample";
      if ((int)label.size() > 17)
         label = label.substr(0, 15) + "..";
      DrawTextNormal(label, (int)(kSampleX + 3), (int)(y + 11), 11);

      mSteps[i].mVolSlider->Draw();
      mSteps[i].mDecaySlider->Draw();
      mSteps[i].mPitchSlider->Draw();
      mSteps[i].mRepeatSlider->Draw();
      mSteps[i].mRandButton->Draw();
      mSteps[i].mClearButton->Draw();
      mSteps[i].mNextButton->Draw();
   }
}

void Tracker::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateTransportListener();
}

void Tracker::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumStepsSlider)
   {
      if (mCurStep >= mNumSteps)
         mCurStep = -1;
      UpdateDimensions();
   }
}

void Tracker::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizeButton)
   {
      RandomizeSteps();
      return;
   }
   //per-step buttons: find which row's dice / delete / next was clicked
   for (int i = 0; i < kMaxSteps; ++i)
   {
      if (button == mSteps[i].mRandButton)
      {
         RandomizeStep(i);
         return;
      }
      if (button == mSteps[i].mClearButton)
      {
         ClearStep(i);
         return;
      }
      if (button == mSteps[i].mNextButton)
      {
         SwitchStepSample(i, 1);
         return;
      }
   }
}

void Tracker::RandomizeStep(int step)
{
   if (step < 0 || step >= kMaxSteps)
      return;
   mSteps[step].mVol = ofRandom(0.2f, 1.0f);
   mSteps[step].mDecayMs = ofRandom(30.0f, 1200.0f);
   mSteps[step].mPitch = (float)(int)ofRandom(-12, 13); //whole semitones, -12..+12
   mSteps[step].mRepeat = 1 + (int)ofRandom(0, 4); //1..4
}

void Tracker::RandomizeSteps()
{
   //randomize every active step (leaves the loaded samples and the step count alone)
   for (int i = 0; i < mNumSteps && i < kMaxSteps; ++i)
      RandomizeStep(i);
}

void Tracker::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void Tracker::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void Tracker::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (int i = 0; i < kMaxSteps; ++i)
   {
      out << mSteps[i].mHasSample;
      if (mSteps[i].mHasSample)
         mSteps[i].mSample.SaveState(out);
   }
}

void Tracker::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   for (int i = 0; i < kMaxSteps; ++i)
   {
      bool has;
      in >> has;
      mSteps[i].mHasSample = has;
      if (has)
         mSteps[i].mSample.LoadState(in);
   }
}

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
//  Transport.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Transport.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ChaosEngine.h"

Transport* TheTransport = nullptr;

//statics
bool Transport::sDoEventLookahead = false;
double Transport::sEventEarlyMs = 150;

Transport::Transport()
{
   assert(TheTransport == nullptr);
   TheTransport = this;

   SetName("transport");

   SetRandomTempo();
}

void Transport::SetRandomTempo()
{
   SetTempo(gRandom() % 80 + 75);
}

void Transport::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTempoSlider = new FloatSlider(this, "tempo", 5, 4, 93, 15, &mTempo, 20, 225);
   mIncreaseTempoButton = new ClickButton(this, " + ", 118, 4);
   mDecreaseTempoButton = new ClickButton(this, " - ", 101, 4);
   mSwingSlider = new FloatSlider(this, "swing", 5, 22, 93, 15, &mSwing, .5f, .7f);
   mSwingIntervalDropdown = new DropdownList(this, "swing interval", 101, 22, &mSwingInterval);
   mTimeSigTopDropdown = new DropdownList(this, "timesigtop", 101, 42, &mTimeSigTop);
   mTimeSigBottomDropdown = new DropdownList(this, "timesigbottom", 101, 60, &mTimeSigBottom);
   mResetButton = new ClickButton(this, "reset", 5, 78);
   mPlayPauseButton = new ClickButton(this, "play/pause", 42, 78, ButtonDisplayStyle::kPause);
   mNudgeBackButton = new ClickButton(this, " < ", 80, 78);
   mNudgeForwardButton = new ClickButton(this, " > ", 110, 78);
   mSetTempoCheckbox = new Checkbox(this, "set tempo", HIDDEN_UICONTROL, HIDDEN_UICONTROL, &mSetTempoBool);

   mTimeSigTopDropdown->AddLabel("2", 2);
   mTimeSigTopDropdown->AddLabel("3", 3);
   mTimeSigTopDropdown->AddLabel("4", 4);
   mTimeSigTopDropdown->AddLabel("5", 5);
   mTimeSigTopDropdown->AddLabel("6", 6);
   mTimeSigTopDropdown->AddLabel("7", 7);
   mTimeSigTopDropdown->AddLabel("8", 8);
   mTimeSigTopDropdown->AddLabel("9", 9);
   mTimeSigTopDropdown->AddLabel("10", 10);
   mTimeSigTopDropdown->AddLabel("11", 11);
   mTimeSigTopDropdown->AddLabel("12", 12);
   mTimeSigTopDropdown->AddLabel("13", 13);
   mTimeSigTopDropdown->AddLabel("14", 14);
   mTimeSigTopDropdown->AddLabel("15", 15);
   mTimeSigTopDropdown->AddLabel("16", 16);
   mTimeSigTopDropdown->AddLabel("17", 17);
   mTimeSigTopDropdown->AddLabel("18", 18);
   mTimeSigTopDropdown->AddLabel("19", 19);

   mTimeSigBottomDropdown->AddLabel("2", 2);
   mTimeSigBottomDropdown->AddLabel("4", 4);
   mTimeSigBottomDropdown->AddLabel("8", 8);
   mTimeSigBottomDropdown->AddLabel("16", 16);

   mSwingIntervalDropdown->AddLabel("4n", 4);
   mSwingIntervalDropdown->AddLabel("8n", 8);
   mSwingIntervalDropdown->AddLabel("16n", 16);
}

void Transport::Init()
{
   IDrawableModule::Init();
}

void Transport::Poll()
{
   if (mWantSetRandomTempo)
   {
      SetRandomTempo();
      mWantSetRandomTempo = false;
   }
}

void Transport::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
}

void Transport::Advance(double ms)
{
   if (mNudgeFactor != 0)
   {
      const float kNudgePower = .05f;
      float nudgeScale = (1 + mNudgeFactor * kNudgePower);

      const float kRamp = .005f;
      if (mNudgeFactor > 0)
         mNudgeFactor = MAX(0, mNudgeFactor - ms * kRamp);
      if (mNudgeFactor < 0)
         mNudgeFactor = MIN(0, mNudgeFactor + ms * kRamp);

      ms *= nudgeScale;
   }

   double amount = ms / MsPerBar();
   assert(amount > 0);

   double oldMeasureTime = mMeasureTime;
   mMeasureTime += amount;
   if (int(mMeasureTime) != int(oldMeasureTime) && mMeasureTime >= mJumpFromMeasure)
   {
      if (mQueuedMeasure != -1)
      {
         SetMeasure(mQueuedMeasure);
         if (mLoopStartMeasure != -1)
            mQueuedMeasure = mLoopStartMeasure;
         else
            mQueuedMeasure = -1;
      }
   }

   if (TheChaosEngine)
      TheChaosEngine->AudioUpdate();

   UpdateListeners(ms);

   for (std::list<IAudioPoller*>::iterator i = mAudioPollers.begin(); i != mAudioPollers.end(); ++i)
   {
      IAudioPoller* poller = *i;
      poller->OnTransportAdvanced(amount);
   }
}

float QuadraticBezier(float x, float a, float b)
{
   // adapted from BEZMATH.PS (1993)
   // by Don Lancaster, SYNERGETICS Inc.
   // http://www.tinaja.com/text/bezmath.html

   float epsilon = 0.00001f;
   a = ofClamp(a, 0, 1);
   b = ofClamp(b, 0, 1);
   if (a == 0.5f)
   {
      a += epsilon;
   }

   // solve t from x (an inverse operation)
   float om2a = 1 - 2 * a;
   float t = (sqrtf(a * a + om2a * x) - a) / om2a;
   float y = (1 - 2 * b) * (t * t) + (2 * b) * t;
   return y;
}

double Transport::Swing(double measurePos)
{
   double swingSlices = double(mSwingInterval) * mTimeSigTop / 4.0;

   double swingPos = measurePos * swingSlices;
   int swingBeat = int(swingPos);
   swingPos -= swingBeat;

   double swung = SwingBeat(swingPos);

   return (swingBeat + swung) / swingSlices;
}

double Transport::SwingBeat(double pos)
{
   double swingDouble = mSwing;
   double term = (.5 - swingDouble) / (swingDouble * swingDouble - swingDouble);
   pos = term * pos * pos + (1 - term) * pos;
   return pos;
}

void Transport::Nudge(double amount)
{
   mNudgeFactor += amount;
}

void Transport::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   double measurePos = GetMeasurePos(gTime);

   int count = int(fmod(mMeasureTime, 1) * mTimeSigTop) + 1;
   std::string display;
   display += ofToString(measurePos, 2) + " " + ofToString(GetMeasure(gTime)) + "\n";
   display += ofToString(count);
   DrawTextNormal(display, 5, 52);

   ofPushStyle();
   float w, h;
   GetDimensions(w, h);
   ofFill();
   ofSetColor(255, 255, 255, 50);
   float beatWidth = w / mTimeSigTop;
   ofRect((count - 1) * beatWidth, 0, beatWidth, h, 0);
   if (count % 2)
      ofSetColor(255, 0, 255);
   else
      ofSetColor(0, 255, 255);
   ofLine(w * measurePos, 0, w * measurePos, h);
   ofRect(0, 95, w * ((measurePos + (GetMeasure(gTime) % 4)) / 4), 5);
   ofPopStyle();

   mSwingSlider->Draw();
   mResetButton->Draw();
   if (TheSynth->IsAudioPaused())
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPlay);
   else
      mPlayPauseButton->SetDisplayStyle(ButtonDisplayStyle::kPause);
   mPlayPauseButton->Draw();
   mTimeSigTopDropdown->Draw();
   mTimeSigBottomDropdown->Draw();
   mSwingIntervalDropdown->Draw();
   mNudgeBackButton->Draw();
   mNudgeForwardButton->Draw();
   mIncreaseTempoButton->Draw();
   mDecreaseTempoButton->Draw();
   mTempoSlider->Draw();

   ofBeginShape();
   for (int i = 0; i < w - 1; ++i)
   {
      float pos = i / float(w - 1);
      float swung = Swing(pos);
      ofVertex(i + 1, h - 1 - swung * (h - 1));
   }
   ofEndShape();
   ofRect(0, h - Swing(measurePos) * h, 4, 1);

   float nudgeMinX = mNudgeBackButton->GetRect(true).getMinX();
   float nudgeMaxX = mNudgeForwardButton->GetRect(true).getMaxX();
   float nudgeX = ofLerp(nudgeMinX, nudgeMaxX, (mNudgeFactor / 15) + .5f);
   ofLine(nudgeX, mNudgeBackButton->GetRect(true).getMinY(), nudgeX, mNudgeBackButton->GetRect(true).getMaxY());
}

void Transport::Reset()
{
   if (mLoopEndMeasure != -1)
      mMeasureTime = mLoopEndMeasure - .01f;
   else
      mMeasureTime = .99f;
   SetQueuedMeasure(NextBufferTime(true), 0);

   if (TheSynth->IsAudioPaused())
      TheSynth->SetAudioPaused(false);
}

void Transport::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
      Reset();
   if (button == mNudgeBackButton)
      Nudge(-1);
   if (button == mNudgeForwardButton)
      Nudge(1);
   if (button == mIncreaseTempoButton)
      SetTempo(floor(mTempo + 1));
   if (button == mDecreaseTempoButton)
      SetTempo(ceil(mTempo - 1));
   if (button == mPlayPauseButton)
      TheSynth->SetAudioPaused(!TheSynth->IsAudioPaused());
}

TransportListenerInfo* Transport::AddListener(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo, bool useEventLookahead)
{
#if DEBUG
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(listener);

   if (module != nullptr)
      assert(module->IsInitialized());
#endif

   //try to update first in case we already point to this
   TransportListenerInfo* info = GetListenerInfo(listener);
   if (info != nullptr)
   {
      info->mInterval = interval;
      info->mOffsetInfo = offsetInfo;
      info->mUseEventLookahead = useEventLookahead;
   }
   else
   {
      mListeners.push_front(TransportListenerInfo(listener, interval, offsetInfo, useEventLookahead));
   }

   return GetListenerInfo(listener);
}

TransportListenerInfo* Transport::GetListenerInfo(ITimeListener* listener)
{
   for (std::list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
   {
      TransportListenerInfo& info = *i;
      if (info.mListener == listener)
         return &info;
   }
   return nullptr;
}

void Transport::RemoveListener(ITimeListener* listener)
{
   for (std::list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end();)
   {
      TransportListenerInfo& info = *i;
      if (info.mListener == listener)
         i = mListeners.erase(i);
      else
         ++i;
   }
}

void Transport::AddAudioPoller(IAudioPoller* poller)
{
#if DEBUG
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(poller);

   if (module != nullptr)
      assert(module->IsInitialized());
#endif

   if (!ListContains(poller, mAudioPollers))
      mAudioPollers.push_front(poller);
}

void Transport::RemoveAudioPoller(IAudioPoller* poller)
{
   mAudioPollers.remove(poller);
}

void Transport::ClearListenersAndPollers()
{
   mListeners.clear();
   mAudioPollers.clear();
}

int Transport::GetQuantized(double time, const TransportListenerInfo* listenerInfo, double* remainderMs /*=nullptr*/)
{
   double offsetMs;
   if (listenerInfo->mOffsetInfo.mOffsetIsInMs)
      offsetMs = listenerInfo->mOffsetInfo.mOffset;
   else
      offsetMs = listenerInfo->mOffsetInfo.mOffset * MsPerBar();
   time += offsetMs;

   int measure = GetMeasure(time);
   double measurePos = GetMeasurePos(time);
   double pos = Swing(measurePos);

   //TODO(Ryan) it seems like most of these cases below could be collapsed into a single case
   //(but there will probably be fallout that needs to be fixed up in various places)
   //changing GetQuantized() to return an unclamped range across the board seems like the right way to go
   NoteInterval interval = listenerInfo->mInterval;
   switch (interval)
   {
      case kInterval_1n:
      case kInterval_2:
      case kInterval_3:
      case kInterval_4:
      case kInterval_8:
      case kInterval_16:
      case kInterval_32:
      case kInterval_64:
      {
         pos *= double(mTimeSigTop) / mTimeSigBottom;
         int ret = measure / (int)GetMeasureFraction(interval);
         if (remainderMs != nullptr)
            *remainderMs = (pos + measure % (int)GetMeasureFraction(interval)) * MsPerBar();
         return ret; //unclamped
      }
      case kInterval_None:
         interval = kInterval_16n; //just pick some default value
         [[fallthrough]];
      case kInterval_2n:
      case kInterval_2nt:
      case kInterval_4n:
      case kInterval_4nt:
      case kInterval_8n:
      case kInterval_8nt:
      case kInterval_16n:
      case kInterval_16nt:
      case kInterval_32n:
      case kInterval_32nt:
      case kInterval_64n:
      {
         pos *= double(mTimeSigTop) / mTimeSigBottom;
         double ret = pos * CountInStandardMeasure(interval);
         if (remainderMs != nullptr)
         {
            double remainder = ret - (int)ret;
            if (mSwing == .5f)
               *remainderMs = remainder * GetDuration(interval);
            else
               *remainderMs = 0; //TODO(Ryan) this is incorrect, figure out how to properly calculate remainderMs when swing is applied
         }
         return (int)ret; //wraps around CountInStandardMeasure(interval) range
      }
      case kInterval_4nd:
      case kInterval_8nd:
      case kInterval_16nd:
      {
         pos *= double(mTimeSigTop) / mTimeSigBottom;
         double ret = (measure + pos) / GetMeasureFraction(interval);
         if (remainderMs != nullptr)
         {
            double remainder = ret - (int)ret;
            if (mSwing == .5f)
               *remainderMs = remainder * GetDuration(interval);
            else
               *remainderMs = 0; //TODO(Ryan) this is incorrect, figure out how to properly calculate remainderMs when swing is applied
         }
         return (int)ret; //unclamped
      }
      case kInterval_CustomDivisor:
      {
         double ret = pos * listenerInfo->mCustomDivisor;
         if (remainderMs != nullptr)
         {
            double remainder = ret - (int)ret;
            if (mSwing == .5f)
               *remainderMs = remainder * (MsPerBar() / listenerInfo->mCustomDivisor);
            else
               *remainderMs = 0; //TODO(Ryan) this is incorrect, figure out how to properly calculate remainderMs when swing is applied
         }
         return (int)ret; //wraps around custom divisor
      }
      default:
         //TODO(Ryan) this doesn't really make sense, does it?
         //assert(false);
         TheSynth->LogEvent("error: GetQuantized() called with invalid interval " + ofToString(interval), kLogEventType_Error);
         return 0;
   }
   return 0;
}

int Transport::CountInStandardMeasure(NoteInterval interval)
{
   switch (interval)
   {
      case kInterval_1n:
         return 1;
      case kInterval_2n:
         return 2;
      case kInterval_2nt:
         return 3;
      case kInterval_4n:
         return 4;
      case kInterval_4nt:
         return 6;
      case kInterval_8n:
         return 8;
      case kInterval_8nt:
         return 12;
      case kInterval_16n:
         return 16;
      case kInterval_16nt:
         return 24;
      case kInterval_32n:
         return 32;
      case kInterval_32nt:
         return 48;
      case kInterval_64n:
         return 64;
      case kInterval_None:
         return 16; //TODO(Ryan) whatever
      default:
         //TODO(Ryan) this doesn't really make sense, does it?
         //assert(false);
         TheSynth->LogEvent("error: CountInStandardMeasure() called with invalid interval " + ofToString(interval), kLogEventType_Error);
         return 1;
   }
   return 0;
}

int Transport::GetStepsPerMeasure(ITimeListener* listener)
{
   TransportListenerInfo* info = GetListenerInfo(listener);
   if (info != nullptr)
   {
      if (info->mInterval == kInterval_CustomDivisor)
         return info->mCustomDivisor;
      return CountInStandardMeasure(info->mInterval) * GetTimeSigTop() / GetTimeSigBottom();
   }
   TheSynth->LogEvent("error: GetStepsPerMeasure() called with unregistered listener", kLogEventType_Error);
   return 8;
}

int Transport::GetSyncedStep(double time, ITimeListener* listener, const TransportListenerInfo* listenerInfo, int length)
{
   double offsetMs;
   if (listenerInfo->mOffsetInfo.mOffsetIsInMs)
      offsetMs = listenerInfo->mOffsetInfo.mOffset;
   else
      offsetMs = listenerInfo->mOffsetInfo.mOffset * MsPerBar();

   int step;
   if (GetMeasureFraction(listenerInfo->mInterval) < 1)
   {
      int stepsPerMeasure = GetStepsPerMeasure(listener);
      int measure = GetMeasure(time + offsetMs);
      step = GetQuantized(time, listenerInfo) + measure * stepsPerMeasure; //GetQuantized() handles offset internally
   }
   else
   {
      int measure = GetMeasure(time + offsetMs);
      step = int(measure / GetMeasureFraction(listenerInfo->mInterval));
   }

   if (length > 0)
      step %= length;

   return step;
}

double Transport::GetDuration(NoteInterval interval)
{
   return MsPerBar() * GetMeasureFraction(interval);
}

double Transport::GetMeasureTimeInternal(double time) const
{
   return mMeasureTime + (time - gTime) / MsPerBar();
}

double Transport::GetMeasureTime(double time) const
{
   double measureTime = GetMeasureTimeInternal(time);
   if (mQueuedMeasure != -1 && measureTime >= mJumpFromMeasure)
      measureTime = mQueuedMeasure + measureTime - mJumpFromMeasure;
   return measureTime;
}

void Transport::SetQueuedMeasure(double time, int measure)
{
   mQueuedMeasure = -1; //clear
   if (mLoopEndMeasure != -1)
      mJumpFromMeasure = mLoopEndMeasure;
   else
      mJumpFromMeasure = GetMeasure(time) + 1;
   mQueuedMeasure = measure;
}

bool Transport::IsPastQueuedMeasureJump(double time) const
{
   double measureTime = GetMeasureTimeInternal(time);
   if (mQueuedMeasure != -1 && measureTime >= mJumpFromMeasure)
      return true;
   return false;
}

double Transport::GetMeasureFraction(NoteInterval interval)
{
   switch (interval)
   {
      case kInterval_1n:
         return 1.0;
      case kInterval_2n:
         return GetMeasureFraction(kInterval_4n) * 2;
      case kInterval_2nt:
         return GetMeasureFraction(kInterval_2n) * 2.0 / 3.0;
      case kInterval_4n:
         return 1.0 / mTimeSigTop;
      case kInterval_4nt:
         return GetMeasureFraction(kInterval_4n) * 2.0 / 3.0;
      case kInterval_8n:
         return GetMeasureFraction(kInterval_4n) * .5;
      case kInterval_8nt:
         return GetMeasureFraction(kInterval_8n) * 2.0 / 3.0;
      case kInterval_16n:
         return GetMeasureFraction(kInterval_4n) * .25;
      case kInterval_16nt:
         return GetMeasureFraction(kInterval_16n) * 2.0 / 3.0;
      case kInterval_32n:
         return GetMeasureFraction(kInterval_4n) * .125;
      case kInterval_32nt:
         return GetMeasureFraction(kInterval_32n) * 2.0 / 3.0;
      case kInterval_64n:
         return GetMeasureFraction(kInterval_4n) * .0625;
      case kInterval_4nd:
         return GetMeasureFraction(kInterval_4n) * 1.5;
      case kInterval_8nd:
         return GetMeasureFraction(kInterval_8n) * 1.5;
      case kInterval_16nd:
         return GetMeasureFraction(kInterval_16n) * 1.5;
      case kInterval_2:
         return 2;
      case kInterval_3:
         return 3;
      case kInterval_4:
         return 4;
      case kInterval_8:
         return 8;
      case kInterval_16:
         return 16;
      case kInterval_32:
         return 32;
      case kInterval_64:
         return 64;
      default:
         return GetMeasureFraction(kInterval_16n);
   }
}

void Transport::UpdateListeners(double jumpMs)
{
   std::list<int> priorities;
   for (const auto& info : mListeners)
   {
      if (info.mListener != nullptr && !ListContains(info.mListener->mTransportPriority, priorities))
         priorities.push_back(info.mListener->mTransportPriority);
   }

   priorities.sort();

   for (const auto& priority : priorities)
   {
      for (const auto& info : mListeners)
      {
         if (info.mListener != nullptr &&
             info.mListener->mTransportPriority == priority &&
             info.mInterval != kInterval_None &&
             info.mInterval != kInterval_Free)
         {
            double lookaheadMs = jumpMs;
            if (info.mUseEventLookahead)
               lookaheadMs = MAX(lookaheadMs, GetEventLookaheadMs());

            double checkTime = gTime + lookaheadMs;

            double remainderMs;
            int oldStep = GetQuantized(checkTime - jumpMs, &info);
            int newStep = GetQuantized(checkTime, &info, &remainderMs);
            bool oldJumped = IsPastQueuedMeasureJump(checkTime - jumpMs);
            bool newJumped = IsPastQueuedMeasureJump(checkTime);
            if (oldStep != newStep ||
                oldJumped != newJumped)
            {
               double time = checkTime - remainderMs + .0001; //TODO(Ryan) investigate this fudge number. I would think that subtracting remainderMs from checkTime would give me a number that gives me the same GetQuantized() result with a zero remainder, but sometimes it is just short of the correct quantization
               /*ofLog() << oldStep << " " << newStep << " " << remainderMs << " " << jumpMs << " " << checkTime << " " << time << " " << GetQuantized(checkTime, info.mInterval) << " " << GetQuantized(time, info.mInterval);
               if (GetQuantized(checkTime + offsetMs, info.mInterval) != GetQuantized(time + offsetMs, info.mInterval))
               {
                  double aboveRemainderMs;
                  GetQuantized(checkTime + offsetMs, info.mInterval, &aboveRemainderMs);
                  double remainderShouldBeZeroMs;
                  GetQuantized(time + offsetMs, info.mInterval, &remainderShouldBeZeroMs);
                  ofLog() << remainderShouldBeZeroMs;
               }*/
               //assert(GetQuantized(checkTime + offsetMs, info.mInterval) == GetQuantized(time + offsetMs, info.mInterval));
               info.mListener->OnTimeEvent(time);
            }
         }
      }
   }
}

void Transport::OnDrumEvent(NoteInterval drumEvent)
{
   for (const auto& info : mListeners)
   {
      if (info.mInterval == drumEvent)
         info.mListener->OnTimeEvent(0); //TODO(Ryan) calc sample offset
   }
}

void Transport::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Transport::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mSetTempoCheckbox)
   {
      if (mSetTempoBool)
      {
         mStartRecordTime = time;
      }
      else if (mStartRecordTime != -1)
      {
         int numBars = 1;
         float recordedTime = time - mStartRecordTime;
         int beats = numBars * GetTimeSigTop();
         float minutes = recordedTime / 1000.0f / 60.0f;
         SetTempo(beats / minutes);
         SetDownbeat();
      }
   }
}

void Transport::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

//static
bool Transport::IsTripletInterval(NoteInterval interval)
{
   if (interval == kInterval_2nt || interval == kInterval_4nt || interval == kInterval_8nt || interval == kInterval_16nt || interval == kInterval_32nt)
      return true;
   return false;
}

void Transport::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("randomize_tempo_on_load", moduleInfo, false);

   SetUpFromSaveData();
}

void Transport::SetUpFromSaveData()
{
   if (mModuleSaveData.GetBool("randomize_tempo_on_load"))
      mWantSetRandomTempo = true;
}

void Transport::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mMeasureTime;
}

void Transport::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev == 0) //load as float instead of double
   {
      float measurePos;
      in >> measurePos;
      mMeasureTime = measurePos;
   }
   else
   {
      in >> mMeasureTime;
   }
}

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
//  Transport.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#pragma once

#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "IAudioPoller.h"

class ITimeListener
{
public:
   virtual ~ITimeListener() {}
   virtual void OnTimeEvent(double time) = 0;
   static constexpr int kDefaultTransportPriority = 100;
   static constexpr int kTransportPriorityEarly = 0;
   static constexpr int kTransportPriorityLate = 200;
   static constexpr int kTransportPriorityVeryEarly = -1000;
   int mTransportPriority{ kDefaultTransportPriority };
};

enum NoteInterval
{
   kInterval_1n,
   kInterval_2n,
   kInterval_2nt,
   kInterval_4n,
   kInterval_4nt,
   kInterval_8n,
   kInterval_8nt,
   kInterval_16n,
   kInterval_16nt,
   kInterval_32n,
   kInterval_32nt,
   kInterval_64n,
   kInterval_4nd,
   kInterval_8nd,
   kInterval_16nd,
   kInterval_2,
   kInterval_3,
   kInterval_4,
   kInterval_8,
   kInterval_16,
   kInterval_32,
   kInterval_64,
   kInterval_Free,
   kInterval_None,
   kInterval_CustomDivisor
};

struct OffsetInfo
{
   OffsetInfo(double offset, bool offsetIsInMs)
   : mOffset(offset)
   , mOffsetIsInMs(offsetIsInMs)
   {}
   double mOffset{ 0 };
   bool mOffsetIsInMs{ false };
};

struct TransportListenerInfo
{
   TransportListenerInfo(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo, bool useEventLookahead)
   : mListener(listener)
   , mInterval(interval)
   , mOffsetInfo(offsetInfo)
   , mUseEventLookahead(useEventLookahead)
   {}

   ITimeListener* mListener{ nullptr };
   NoteInterval mInterval{ NoteInterval::kInterval_None };
   OffsetInfo mOffsetInfo;
   bool mUseEventLookahead{ false };
   int mCustomDivisor{ 8 };
};

class Transport : public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   Transport();

   void CreateUIControls() override;
   void Poll() override;

   float GetTempo() { return mTempo; }
   void SetTempo(float tempo) { mTempo = tempo; }
   void SetTimeSignature(int top, int bottom)
   {
      mTimeSigTop = top;
      mTimeSigBottom = bottom;
   }
   int GetTimeSigTop() { return mTimeSigTop; }
   int GetTimeSigBottom() { return mTimeSigBottom; }
   void SetSwing(float swing) { mSwing = swing; }
   float GetSwing() { return mSwing; }
   double MsPerBar() const { return 60.0 / mTempo * 1000 * mTimeSigTop * 4.0 / mTimeSigBottom; }
   void Advance(double ms);
   TransportListenerInfo* AddListener(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo, bool useEventLookahead);
   void RemoveListener(ITimeListener* listener);
   TransportListenerInfo* GetListenerInfo(ITimeListener* listener);
   void AddAudioPoller(IAudioPoller* poller);
   void RemoveAudioPoller(IAudioPoller* poller);
   void ClearListenersAndPollers();
   double GetDuration(NoteInterval interval);
   int GetQuantized(double time, const TransportListenerInfo* listenerInfo, double* remainderMs = nullptr);
   double GetMeasurePos(double time) const { return fmod(GetMeasureTime(time), 1); }
   void SetMeasureTime(double measureTime) { mMeasureTime = measureTime; }
   int GetMeasure(double time) const { return (int)floor(GetMeasureTime(time)); }
   double GetMeasureTime(double time) const;
   void SetMeasure(int count) { mMeasureTime = mMeasureTime - (int)mMeasureTime + count; }
   void SetDownbeat() { mMeasureTime = mMeasureTime - (int)mMeasureTime - .001; }
   static int CountInStandardMeasure(NoteInterval interval);
   void Reset();
   void OnDrumEvent(NoteInterval drumEvent);
   void SetLoop(int measureStart, int measureEnd)
   {
      assert(measureStart < measureEnd);
      mLoopStartMeasure = measureStart;
      mLoopEndMeasure = measureEnd;
      mQueuedMeasure = measureStart;
      mJumpFromMeasure = measureEnd;
   }
   void ClearLoop()
   {
      mLoopStartMeasure = -1;
      mLoopEndMeasure = -1;
      mQueuedMeasure = -1;
   }
   void SetQueuedMeasure(double time, int measure);
   bool IsPastQueuedMeasureJump(double time) const;
   double GetMeasureFraction(NoteInterval interval);
   int GetStepsPerMeasure(ITimeListener* listener);
   int GetSyncedStep(double time, ITimeListener* listener, const TransportListenerInfo* listenerInfo, int length = -1);

   bool CheckNeedsDraw() override { return true; }

   double GetEventLookaheadMs() { return sDoEventLookahead ? sEventEarlyMs : 0; }

   //IDrawableModule
   void Init() override;
   void KeyPressed(int key, bool isRepeat) override;
   bool IsSingleton() const override { return true; }

   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   static bool sDoEventLookahead;
   static double sEventEarlyMs;

   bool IsEnabled() const override { return true; }

   static bool IsTripletInterval(NoteInterval interval);

private:
   void UpdateListeners(double jumpMs);
   double Swing(double measurePos);
   double SwingBeat(double pos);
   void Nudge(double amount);
   void SetRandomTempo();
   double GetMeasureTimeInternal(double time) const;

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 140;
      height = 100;
   }

   float mTempo{ 120 };
   int mTimeSigTop{ 4 };
   int mTimeSigBottom{ 4 };
   double mMeasureTime{ 0 };
   int mSwingInterval{ 8 };
   float mSwing{ .5 };
   FloatSlider* mSwingSlider{ nullptr };
   ClickButton* mResetButton{ nullptr };
   ClickButton* mPlayPauseButton{ nullptr };
   DropdownList* mTimeSigTopDropdown{ nullptr };
   DropdownList* mTimeSigBottomDropdown{ nullptr };
   DropdownList* mSwingIntervalDropdown{ nullptr };
   bool mSetTempoBool{ false };
   Checkbox* mSetTempoCheckbox{ nullptr };
   double mStartRecordTime{ -1 };
   ClickButton* mNudgeBackButton{ nullptr };
   ClickButton* mNudgeForwardButton{ nullptr };
   ClickButton* mIncreaseTempoButton{ nullptr };
   ClickButton* mDecreaseTempoButton{ nullptr };
   FloatSlider* mTempoSlider{ nullptr };
   int mLoopStartMeasure{ -1 };
   int mLoopEndMeasure{ -1 };
   int mQueuedMeasure{ -1 };
   int mJumpFromMeasure{ -1 };
   bool mWantSetRandomTempo{ false };
   float mNudgeFactor{ 0 };

   std::list<TransportListenerInfo> mListeners;
   std::list<IAudioPoller*> mAudioPollers;
};

extern Transport* TheTransport;

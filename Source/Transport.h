//
//  Transport.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__Transport__
#define __modularSynth__Transport__

#include <iostream>
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
   kInterval_None
};

struct OffsetInfo
{
   OffsetInfo(double offset, bool offsetIsInMs)
   : mOffset(offset), mOffsetIsInMs(offsetIsInMs) {}
   double mOffset;
   bool mOffsetIsInMs;
};

struct TransportListenerInfo
{
   TransportListenerInfo(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo, bool useEventLookahead)
   : mListener(listener), mInterval(interval), mOffsetInfo(offsetInfo), mUseEventLookahead(useEventLookahead) {}
   
   ITimeListener* mListener;
   NoteInterval mInterval;
   OffsetInfo mOffsetInfo;
   bool mUseEventLookahead;
};

class Transport : public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   Transport();
   
   string GetTitleLabel() override { return "transport"; }
   void CreateUIControls() override;

   float GetTempo() { return mTempo; }
   void SetTempo(float tempo) { mTempo = tempo; }
   void SetTimeSignature(int top, int bottom) { mTimeSigTop = top; mTimeSigBottom = bottom; }
   int GetTimeSigTop() { return mTimeSigTop; }
   int GetTimeSigBottom() { return mTimeSigBottom; }
   void SetSwing(float swing) { mSwing = swing; }
   float GetSwing() { return mSwing; }
   double MsPerBar() const { return 60/mTempo * 1000 * mTimeSigTop * 4.0f/mTimeSigBottom; }
   void Advance(double ms);
   void AddListener(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo, bool useEventLookahead);
   void RemoveListener(ITimeListener* listener);
   bool UpdateListener(ITimeListener* listener, NoteInterval interval);
   bool UpdateListener(ITimeListener* listener, NoteInterval interval, OffsetInfo offsetInfo);
   void AddAudioPoller(IAudioPoller* poller);
   void RemoveAudioPoller(IAudioPoller* poller);
   double GetDuration(NoteInterval interval);
   int GetQuantized(double time, NoteInterval interval, double* remainderMs = nullptr);
   double GetMeasurePos(double time) const { return fmod(GetMeasureTime(time), 1); }
   void SetMeasurePos(double pos) { mMeasureTime = mMeasureTime - (int)mMeasureTime + pos; }
   int GetMeasure(double time) const { return (int)GetMeasureTime(time); }
   double GetMeasureTime(double time) const { return mMeasureTime + (time - gTime) / MsPerBar(); }
   void SetMeasure(int count) { mMeasureTime = mMeasureTime - (int)mMeasureTime + count; }
   void SetDownbeat() { mMeasureTime = mMeasureTime - (int)mMeasureTime - .001; }
   static int CountInStandardMeasure(NoteInterval interval);
   void Reset();
   void OnDrumEvent(NoteInterval drumEvent);
   void SetLoop(int measureStart, int measureEnd) { assert(measureStart < measureEnd); mLoopStartMeasure = measureStart; mLoopEndMeasure = measureEnd; }
   void ClearLoop() { mLoopStartMeasure = -1; mLoopEndMeasure = -1; }
   
   bool CheckNeedsDraw() override { return true; }
   
   double GetEventLookaheadMs() { return sDoEventLookahead ? sEventEarlyMs : 0; }
   
   //IDrawableModule
   void Init() override;
   void KeyPressed(int key, bool isRepeat) override;
   bool IsSingleton() const override { return true; }

   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   static bool sDoEventLookahead;
   static double sEventEarlyMs;
   
private:
   void UpdateListeners(double jumpMs);
   float Swing(float measurePos);
   float SwingBeat(float pos);
   void Nudge(float amount);
   void AdjustTempo(double amount);
   double GetMeasureFraction(NoteInterval interval);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 140; height = 100; }
   bool Enabled() const override { return true; }
   
   float mTempo;
   int mTimeSigTop;
   int mTimeSigBottom;
   double mMeasureTime;
   int mSwingInterval;
   float mSwing;
   FloatSlider* mSwingSlider;
   ClickButton* mResetButton;
   DropdownList* mTimeSigTopDropdown;
   DropdownList* mTimeSigBottomDropdown;
   DropdownList* mSwingIntervalDropdown;
   bool mSetTempoBool;
   Checkbox* mSetTempoCheckbox;
   double mStartRecordTime;
   ClickButton* mNudgeBackButton;
   ClickButton* mNudgeForwardButton;
   ClickButton* mIncreaseTempoButton;
   ClickButton* mDecreaseTempoButton;
   FloatSlider* mTempoSlider;
   int mLoopStartMeasure;
   int mLoopEndMeasure;

   list<TransportListenerInfo> mListeners;
   list<IAudioPoller*> mAudioPollers;
};

extern Transport* TheTransport;

#endif /* defined(__modularSynth__Transport__) */


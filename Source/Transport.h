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
   virtual void OnTimeEvent(int samplesTo) = 0;
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
   kInterval_Kick,
   kInterval_Snare,
   kInterval_Hat,
   kInterval_Free,
   kInterval_None
};

struct TransportListenerInfo
{
   TransportListenerInfo(ITimeListener* listener, NoteInterval interval, float offset = 0, bool offsetIsInMs = true)
   : mListener(listener), mInterval(interval), mOffset(offset), mOffsetIsInMs(offsetIsInMs) {}
   
   ITimeListener* mListener;
   NoteInterval mInterval;
   float mOffset;
   bool mOffsetIsInMs;
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
   float MsPerBar() const;
   void Advance(float ms);
   void AddListener(ITimeListener* listener, NoteInterval interval, float offset = 0, bool offsetIsInMs = true);
   void RemoveListener(ITimeListener* listener);
   bool UpdateListener(ITimeListener* listener, NoteInterval interval, float offset = 0, bool offsetIsInMs = true);
   void AddAudioPoller(IAudioPoller* poller);
   void RemoveAudioPoller(IAudioPoller* poller);
   float GetDuration(NoteInterval interval);
   int GetQuantized(float offsetMs, NoteInterval interval);
   float GetMeasurePos() const { return mMeasurePos; }
   float GetMeasurePos(int offset) const;
   void SetMeasurePos(float pos) { mMeasurePos = pos; }
   int GetMeasure() const { return mMeasureCount; }
   void SetMeasure(int count) { mMeasureCount = count; }
   void SetDownbeat() { mMeasurePos = .999f; --mMeasureCount; }
   static int CountInStandardMeasure(NoteInterval interval);
   void Reset();
   void OnDrumEvent(NoteInterval drumEvent);
   void SetLoop(int measureStart, int measureEnd) { assert(measureStart < measureEnd); mLoopStartMeasure = measureStart; mLoopEndMeasure = measureEnd; }
   void ClearLoop() { mLoopStartMeasure = -1; mLoopEndMeasure = -1; }
   
   bool CheckNeedsDraw() override { return true; }
   
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
private:
   void UpdateListeners(float jumpMs);
   float Swing(float measurePos);
   float SwingBeat(float pos);
   void Nudge(float amount);
   void AdjustTempo(float amount);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 140; height = 100; }
   bool Enabled() const override { return true; }
   
   float mTempo;
   int mTimeSigTop;
   int mTimeSigBottom;
   int mQueuedTimeSigTop;
   int mQueuedTimeSigBottom;
   bool mTimeSigChangeQueued;
   unsigned int mMeasureCount;
   float mMeasurePos;
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


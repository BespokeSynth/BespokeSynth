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
//  LooperRecorder.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/9/12.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "RollingBuffer.h"
#include "RadioButton.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Looper.h"
#include "Ramp.h"
#include "DropdownList.h"
#include "Push2Control.h"

class Stutter;
class PatchCableSource;

class LooperRecorder : public IAudioProcessor, public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IRadioButtonListener, public IIntSliderListener, public IDropdownListener, public IPush2GridController
{
public:
   LooperRecorder();
   ~LooperRecorder();
   static IDrawableModule* Create() { return new LooperRecorder(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void SetNumBars(int numBars) { mNumBars = numBars; }
   int GetNumBars() const { return mNumBars; }
   void Commit(Looper* looper);
   void RequestMerge(Looper* looper);
   void RequestSwap(Looper* looper);
   void RequestCopy(Looper* looper);
   Looper* GetMergeSource() { return mMergeSource; }
   Looper* GetSwapSource() { return mSwapSource; }
   Looper* GetCopySource() { return mCopySource; }
   int IncreaseCommitCount() { return ++mCommitCount; }
   void RemoveLooper(Looper* looper);
   void ResetSpeed();
   RollingBuffer* GetRecordBuffer() { return &mRecordBuffer; }
   Looper* GetNextCommitTarget() { return (mNextCommitTargetIndex < (int)mLoopers.size()) ? mLoopers[mNextCommitTargetIndex] : nullptr; }

   void StartFreeRecord(double time);
   void EndFreeRecord(double time);
   void CancelFreeRecord();

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IDrawableModule
   void Poll() override;
   void PreRepatch(PatchCableSource* cableSource) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   bool HasDebugDraw() const override { return true; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   void SyncLoopLengths();
   void UpdateSpeed();
   float AdjustedRootForSpeed();
   void SnapToClosestPitch();
   void Resample(bool setKey);
   void DrawCircleHash(ofVec2f center, float progress, float width, float innerRadius, float outerRadius);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   static constexpr int kMaxLoopers = 8;

   float mWidth{ 235 };
   float mHeight{ 126 };
   RollingBuffer mRecordBuffer;
   std::array<Looper*, kMaxLoopers> mLoopers{ nullptr };
   int mNumLoopers{ 4 };
   int mNumBars{ 1 };
   DropdownList* mNumBarsSelector{ nullptr };
   float mSpeed{ 1 };
   float mBaseTempo{ 120 };
   ClickButton* mResampleButton{ nullptr };
   ClickButton* mResampAndSetButton{ nullptr };
   Looper* mMergeSource{ nullptr };
   Looper* mSwapSource{ nullptr };
   Looper* mCopySource{ nullptr };
   int mCommitCount{ 0 };
   ClickButton* mDoubleTempoButton{ nullptr };
   ClickButton* mHalfTempoButton{ nullptr };
   ClickButton* mShiftMeasureButton{ nullptr };
   ClickButton* mHalfShiftButton{ nullptr };
   ClickButton* mClearOverdubButton{ nullptr };
   Ramp mQuietInputRamp;
   double mUnquietInputTime{ -1 };
   ClickButton* mShiftDownbeatButton{ nullptr };
   ClickButton* mOrigSpeedButton{ nullptr };
   ClickButton* mSnapPitchButton{ nullptr };
   float mCommitDelay{ 0 };
   FloatSlider* mCommitDelaySlider{ nullptr };
   ChannelBuffer mWriteBuffer;
   Looper* mCommitToLooper{ nullptr };
   std::array<PatchCableSource*, kMaxLoopers> mLooperPatchCables{ nullptr };
   ClickButton* mCommit1BarButton{ nullptr };
   ClickButton* mCommit2BarsButton{ nullptr };
   ClickButton* mCommit4BarsButton{ nullptr };
   ClickButton* mCommit8BarsButton{ nullptr };
   IntSlider* mNextCommitTargetSlider{ nullptr };
   int mNextCommitTargetIndex{ 0 };
   Checkbox* mAutoAdvanceThroughLoopersCheckbox{ nullptr };
   bool mAutoAdvanceThroughLoopers{ false };
   bool mTemporarilySilenceAfterCommit{ false };
   std::array<Checkbox*, kMaxLoopers> mWriteForLooperCheckbox{ nullptr };
   std::array<bool, kMaxLoopers> mWriteForLooper{ false };
   std::array<double, kMaxLoopers> mStartRecordMeasureTime{ 0 };
   float mLatencyFixMs{ 0 };
   FloatSlider* mLatencyFixMsSlider{ nullptr };

   bool mFreeRecording{ false };
   Checkbox* mFreeRecordingCheckbox{ nullptr };
   double mStartFreeRecordTime{ 0 };
   ClickButton* mCancelFreeRecordButton{ nullptr };

   enum RecorderMode
   {
      kRecorderMode_Record,
      kRecorderMode_Overdub,
      kRecorderMode_Loop
   };
   RecorderMode mRecorderMode{ RecorderMode::kRecorderMode_Record };
   DropdownList* mModeSelector{ nullptr };
};

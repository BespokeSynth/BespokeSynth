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

#ifndef __modularSynth__LooperRecorder__
#define __modularSynth__LooperRecorder__

#include <iostream>
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

class Stutter;
class PatchCableSource;

class LooperRecorder : public IAudioProcessor, public IDrawableModule, public IButtonListener, public IFloatSliderListener, public IRadioButtonListener, public IIntSliderListener, public IDropdownListener
{
public:
   LooperRecorder();
   ~LooperRecorder();
   static IDrawableModule* Create() { return new LooperRecorder(); }
   
   string GetTitleLabel() override { return "looper recorder"; }
   void CreateUIControls() override;

   void Init() override;
   void SetNumBars(int numBars) { mNumBars = numBars; }
   int NumBars() { return mNumBars; }
   void Commit(Looper* looper);
   void RequestMerge(Looper* looper);
   void RequestSwap(Looper* looper);
   void RequestCopy(Looper* looper);
   Looper* GetMergeSource() { return mMergeSource; }
   Looper* GetSwapSource() { return mSwapSource; }
   Looper* GetCopySource() { return mCopySource; }
   int IncreaseCommitCount() { return ++mCommitCount; }
   void RemoveLooper(Looper* looper);
   void SetHeadphonesTarget(IAudioReceiver* target) { mHeadphonesTarget = target; }
   void SetOutputTarget(IAudioReceiver* target) { mOutputTarget = target; }
   void ResetSpeed();
   float GetCommitDelay() { return mCommitDelay; }
   RollingBuffer* GetRecordBuffer() { return &mRecordBuffer; }
   Looper* GetNextCommitTarget() { return (mNextCommitTargetIndex < (int)mLoopers.size()) ? mLoopers[mNextCommitTargetIndex] : nullptr; }
   
   void StartFreeRecord();
   void EndFreeRecord();
   void CancelFreeRecord();
   bool InFreeRecord() { return mFreeRecording; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IDrawableModule
   void KeyPressed(int key, bool isRepeat) override;
   void Poll() override;
   void PreRepatch(PatchCableSource* cableSource) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   bool HasDebugDraw() const override { return true; }
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void SyncLoopLengths();
   void UpdateSpeed();
   float AdjustedRootForSpeed();
   void SnapToClosestPitch();
   void Resample(bool setKey);
   void DrawCircleHash(ofVec2f center, float progress, float width, float innerRadius, float outerRadius);
   void SyncCablesToLoopers();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   RollingBuffer mRecordBuffer;
   vector<Looper*> mLoopers;
   int mNumBars;
   DropdownList* mNumBarsSelector;
   float mSpeed;
   float mBaseTempo;
   ClickButton* mResampleButton;
   ClickButton* mResampAndSetButton;
   Looper* mMergeSource;
   Looper* mSwapSource;
   Looper* mCopySource;
   int mCommitCount;
   ClickButton* mDoubleTempoButton;
   ClickButton* mHalfTempoButton;
   ClickButton* mShiftMeasureButton;
   ClickButton* mHalfShiftButton;
   ClickButton* mClearOverdubButton;
   Ramp mQuietInputRamp;
   double mUnquietInputTime;
   ClickButton* mShiftDownbeatButton;
   ClickButton* mOrigSpeedButton;
   ClickButton* mSnapPitchButton;
   IAudioReceiver* mHeadphonesTarget;
   IAudioReceiver* mOutputTarget;
   float mCommitDelay;
   FloatSlider* mCommitDelaySlider;
   ChannelBuffer mWriteBuffer;
   Looper* mCommitToLooper;
   vector<PatchCableSource*> mLooperPatchCables;
   ClickButton* mCommit1BarButton;
   ClickButton* mCommit2BarsButton;
   ClickButton* mCommit4BarsButton;
   ClickButton* mCommit8BarsButton;
   IntSlider* mNextCommitTargetSlider;
   int mNextCommitTargetIndex;
   
   bool mFreeRecording;
   Checkbox* mFreeRecordingCheckbox;
   double mStartFreeRecordTime;
   ClickButton* mCancelFreeRecordButton;
   
   enum RecorderMode
   {
      kRecorderMode_Record,
      kRecorderMode_Overdub,
      kRecorderMode_Loop
   };
   RecorderMode mRecorderMode;
   DropdownList* mModeSelector;
};


#endif /* defined(__modularSynth__LooperRecorder__) */


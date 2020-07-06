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
   void DoCommit();
   void SyncLoopLengths();
   void UpdateSpeed();
   float AdjustedRootForSpeed();
   void SnapToClosestPitch();
   void Resample(bool setKey);
   void DrawCircleHash(ofVec2f center, float progress, float width, float innerRadius, float outerRadius);
   void SyncCablesToLoopers();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 235; height = 124; }
   bool Enabled() const override { return mEnabled; }
   
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
   double mBarRecordTime;
   ClickButton* mBarRecordButton;
   
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
   RadioButton* mModeSelector;
};


#endif /* defined(__modularSynth__LooperRecorder__) */


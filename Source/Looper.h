//
//  Looper.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#ifndef __modularSynth__Looper__
#define __modularSynth__Looper__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "Ramp.h"
#include "Granulator.h"
#include "JumpBlender.h"
#include "PitchShifter.h"
#include "INoteReceiver.h"

class LooperRecorder;
class Rewriter;
class Sample;

#define LOOPER_COMMIT_FADE_SAMPLES 200

class Looper : public IAudioProcessor, public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener, public IRadioButtonListener, public INoteReceiver
{
public:
   Looper();
   ~Looper();
   static IDrawableModule* Create() { return new Looper(); }
   
   string GetTitleLabel() override { return "looper"; }
   void CreateUIControls() override;

   void SetRecorder(LooperRecorder* recorder);
   void Clear();
   void Commit(RollingBuffer* commitBuffer = nullptr);
   void Fill(ChannelBuffer* buffer, int length);
   void ResampleForNewSpeed();
   int NumBars() const { return mNumBars; }
   int GetRecorderNumBars() const;
   void SetNumBars(int numBars);
   void SetSpeed(float speed) { mSpeed = speed; }
   void RecalcLoopLength() { UpdateNumBars(mNumBars); }
   void DoubleNumBars() { mNumBars *= 2; }
   void HalveNumBars();
   void ShiftMeasure() { mWantShiftMeasure = true; }
   void HalfShift() { mWantHalfShift = true; }
   void ShiftDownbeat() { mWantShiftDownbeat = true; }
   ChannelBuffer* GetLoopBuffer(int& loopLength);
   void SetLoopBuffer(ChannelBuffer* buffer);
   void LockBufferMutex() { mBufferMutex.lock(); }
   void UnlockBufferMutex() { mBufferMutex.unlock(); }
   void SampleDropped(int x, int y, Sample* sample) override;
   
   void Poll() override;
   void Exit() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;

   void MergeIn(Looper* otherLooper);
   void SwapBuffers(Looper* otherLooper);
   void CopyBuffer(Looper* sourceLooper);
   void SetLoopLength(int length);
   void SetRewriter(Rewriter* rewriter) { mRewriter = rewriter; }
   void Rewrite();
   
   bool CheckNeedsDraw() override { return true; }
   
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
private:
   void DoShiftMeasure();
   void DoHalfShift();
   void DoShiftDownbeat();
   void DoShiftOffset();
   void DoCommit();
   void UpdateNumBars(int oldNumBars);
   void BakeVolume();
   void DoUndo();
   void ProcessFourTet(double time, int sampleIdx);
   void ProcessScratch();
   void ProcessGranular(double time, float bufferOffset, float* output);
   void ProcessBeatwheel(double time, int sampleIdx);
   int GetMeasureSliceIndex(double time, int sampleIdx, int slicesPerBar);
   void DrawBeatwheel();
   float GetActualLoopPos(int samplesIn) const;
   int GetBeatwheelDepthLevel() const;
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   static const int BUFFER_X = 3;
   static const int BUFFER_Y = 3;
   static const int BUFFER_W = 170;
   static const int BUFFER_H = 93;

   ChannelBuffer* mBuffer;
   ChannelBuffer mWorkBuffer;
   int mLoopLength;
   float mLoopPos;
   RollingBuffer* mRecordBuffer;
   int mNumBars;
   ClickButton* mClearButton;
   DropdownList* mNumBarsSelector;
   float mVol;
   float mSmoothedVol;
   FloatSlider* mVolSlider;
   float mSpeed;
   LooperRecorder* mRecorder;
   ClickButton* mMergeButton;
   ClickButton* mSwapButton;
   ClickButton* mCopyButton;
   RollingBuffer* mCommitBuffer; //if this is set, a commit happens next audio frame
   ClickButton* mVolumeBakeButton;
   bool mWantBakeVolume;
   int mLastCommit;
   ClickButton* mSaveButton;
   bool mWantShiftMeasure;
   bool mWantHalfShift;
   bool mWantShiftDownbeat;
   bool mWantShiftOffset;
   bool mMute;
   Checkbox* mMuteCheckbox;
   ClickButton* mCommitButton;
   ClickButton* mDoubleSpeedButton;
   ClickButton* mHalveSpeedButton;
   ChannelBuffer* mUndoBuffer;
   ClickButton* mUndoButton;
   bool mWantUndo;
   bool mReplaceOnCommit;
   float mLoopPosOffset;
   FloatSlider* mLoopPosOffsetSlider;
   ClickButton* mResetOffsetButton;
   bool mAllowChop;
   Checkbox* mAllowChopCheckbox;
   int mChopMeasure;
   ClickButton* mWriteOffsetButton;
   float mScratchSpeed;
   bool mAllowScratch;
   FloatSlider* mScratchSpeedSlider;
   Checkbox* mAllowScratchCheckbox;
   double mLastCommitTime;
   float mFourTet;
   FloatSlider* mFourTetSlider;
   int mFourTetSlices;
   DropdownList* mFourTetSlicesDropdown;
   ofMutex mBufferMutex;
   Ramp mMuteRamp;
   JumpBlender mJumpBlender[ChannelBuffer::kMaxNumChannels];
   bool mClearCommitBuffer;
   Rewriter* mRewriter;
   bool mWantRewrite;
   int mLoopCount;
   ChannelBuffer* mQueuedNewBuffer;
   float mDecay;
   FloatSlider* mDecaySlider;
   bool mWriteInput;
   Checkbox* mWriteInputCheckbox;
   ClickButton* mQueueCaptureButton;
   bool mCaptureQueued;
   Ramp mWriteInputRamp;
   float mLastInputSample[ChannelBuffer::kMaxNumChannels];

   //granular
   bool mShowGranular;
   Checkbox* mShowGranularCheckbox;
   bool mGranular;
   Checkbox* mGranularCheckbox;
   Granulator mGranulator;
   FloatSlider* mGranOverlap;
   FloatSlider* mGranSpeed;
   FloatSlider* mGranLengthMs;
   FloatSlider* mPosSlider;
   bool mPausePos;
   Checkbox* mPausePosCheckbox;
   FloatSlider* mGranPosRandomize;
   FloatSlider* mGranSpeedRandomize;
   Checkbox* mGranOctaveCheckbox;
   
   //beatwheel
   bool mBeatwheel;
   static float mBeatwheelPosRight;
   static float mBeatwheelDepthRight;
   static float mBeatwheelPosLeft;
   static float mBeatwheelDepthLeft;
   Checkbox* mBeatwheelCheckbox;
   FloatSlider* mBeatwheelPosRightSlider;
   FloatSlider* mBeatwheelDepthRightSlider;
   FloatSlider* mBeatwheelPosLeftSlider;
   FloatSlider* mBeatwheelDepthLeftSlider;
   bool mBeatwheelControlFlip;
   static bool mBeatwheelSingleMeasure;
   Checkbox* mBeatwheelSingleMeasureCheckbox;

   //pitch shifter
   PitchShifter* mPitchShifter[ChannelBuffer::kMaxNumChannels];
   float mPitchShift;
   FloatSlider* mPitchShiftSlider;
   bool mKeepPitch;
   Checkbox* mKeepPitchCheckbox;
};


#endif /* defined(__modularSynth__Looper__) */


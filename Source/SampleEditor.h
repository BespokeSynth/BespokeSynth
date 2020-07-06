//
//  SampleEditor.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#ifndef __modularSynth__SampleEditor__
#define __modularSynth__SampleEditor__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "PitchShifter.h"

class SampleBank;
class Sample;

class SampleEditor : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener, public INoteReceiver
{
public:
   SampleEditor();
   ~SampleEditor();
   static IDrawableModule* Create() { return new SampleEditor(); }
   
   string GetTitleLabel() override { return "sample editor"; }
   void CreateUIControls() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void ResetBar() { mReset = true; mCurrentBar = 0;}
   void SetTransposition(float transposition) { mTransposition = transposition; }
   void SetSampleIndex(int index) { mSampleIndex = index; UpdateSample(); }
   void Play() { mPlay = true; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void OnTimeEvent(double time) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   void RecalcPos();
   void UpdateSample();
   void UpdateSampleList();
   void UpdateBPM();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;

   SampleBank* mBank;
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   int mSampleIndex;
   DropdownList* mSampleList;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   int mCurrentBar;
   int mMeasureEarly;
   bool mEditMode;
   Checkbox* mEditCheckbox;
   float mSampleStart;
   FloatSlider* mSampleStartSlider;
   float mSampleEnd;
   FloatSlider* mSampleEndSlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mNumBars;
   IntSlider* mNumBarsSlider;
   ClickButton* mEditModeStart;
   ClickButton* mPadSampleButton;
   ClickButton* mWriteButton;
   float mOriginalBpm;
   bool mKeepPitch;
   Checkbox* mKeepPitchCheckbox;
   float mPitchShift;
   FloatSlider* mPitchShiftSlider;
   PitchShifter* mPitchShifter[ChannelBuffer::kMaxNumChannels];
   PatchCableSource* mSampleBankCable;
   bool mReset;
   float mTransposition;
   float mPlayPosition;
   
   ChannelBuffer mDrawBuffer;
   int mDrawBufferLength;
};

#endif /* defined(__modularSynth__SampleEditor__) */


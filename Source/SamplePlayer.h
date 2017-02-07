//
//  SamplePlayer.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#ifndef __modularSynth__SamplePlayer__
#define __modularSynth__SamplePlayer__

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

class SamplePlayer : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener, public INoteReceiver
{
public:
   SamplePlayer();
   ~SamplePlayer();
   static IDrawableModule* Create() { return new SamplePlayer(); }
   
   string GetTitleLabel() override { return "sample"; }
   void CreateUIControls() override;

   void PostRepatch(PatchCableSource* cable) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
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
   void OnTimeEvent(int samplesTo) override;
   
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
   void GetModuleDimensions(int& x, int& y) override;
   void OnClicked(int x, int y, bool right) override;

   SampleBank* mBank;
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
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
   PitchShifter mPitchShifter;
   PatchCableSource* mSampleBankCable;
   bool mReset;
   float mTransposition;
   float mPlayPosition;
   
   float* mDrawBuffer;
   int mDrawBufferLength;
};

#endif /* defined(__modularSynth__SamplePlayer__) */


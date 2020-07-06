//
//  SampleFinder.h
//  modularSynth
//
//  Created by Ryan Challinor on 6/18/13.
//
//

#ifndef __modularSynth__SampleFinder__
#define __modularSynth__SampleFinder__

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
#include "SampleDrawer.h"

class Sample;

class SampleFinder : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   SampleFinder();
   ~SampleFinder();
   static IDrawableModule* Create() { return new SampleFinder(); }
   
   string GetTitleLabel() override { return "sample finder"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;
   
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void UpdateSample();
   void DoWrite();
   void UpdateZoomExtents();
   float GetSpeed();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   int mMeasureEarly;
   bool mEditMode;
   Checkbox* mEditCheckbox;
   int mClipStart;
   IntSlider* mClipStartSlider;
   int mClipEnd;
   IntSlider* mClipEndSlider;
   float mZoomStart;
   float mZoomEnd;
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mNumBars;
   IntSlider* mNumBarsSlider;
   ClickButton* mWriteButton;
   double mPlayhead;
   bool mWantWrite;
   ClickButton* mDoubleLengthButton;
   ClickButton* mHalveLengthButton;
   SampleDrawer mSampleDrawer;
   bool mReverse;
   Checkbox* mReverseCheckbox;
};

#endif /* defined(__modularSynth__SampleFinder__) */


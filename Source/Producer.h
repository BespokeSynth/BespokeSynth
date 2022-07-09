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
//  Producer.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/13/13.
//
//

#ifndef __modularSynth__Producer__
#define __modularSynth__Producer__

#include <iostream>
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
#include "BiquadFilterEffect.h"

#define PRODUCER_NUM_BIQUADS 5

class Sample;

class Producer : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   Producer();
   ~Producer();
   static IDrawableModule* Create() { return new Producer(); }


   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;


   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;


private:
   void UpdateSample();
   void DoWrite();
   void UpdateZoomExtents();
   int GetMeasureSample(int measure);
   float GetBufferPos(int sample);
   int GetMeasureForSample(int sample);
   int GetSamplesPerMeasure();
   bool IsSkipMeasure(int measure);

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   Sample* mSample;

   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   float mClipStart;
   FloatSlider* mClipStartSlider;
   float mClipEnd;
   FloatSlider* mClipEndSlider;
   float mZoomStart;
   FloatSlider* mZoomStartSlider;
   float mZoomEnd;
   FloatSlider* mZoomEndSlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mNumBars;
   IntSlider* mNumBarsSlider;
   ClickButton* mWriteButton;
   int mPlayhead;
   float mTempo;
   FloatSlider* mTempoSlider;
   int mStartOffset;
   ClickButton* mCalcTempoButton;
   IntSlider* mStartOffsetSlider;
   ClickButton* mDoubleLengthButton;
   ClickButton* mHalveLengthButton;
   BiquadFilterEffect mBiquad[PRODUCER_NUM_BIQUADS];
   std::list<int> mSkipMeasures;
   ClickButton* mRestartButton;
};


#endif /* defined(__modularSynth__Producer__) */

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

#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;


   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

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
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   Sample* mSample{ nullptr };

   float mVolume{ .6 };
   FloatSlider* mVolumeSlider{ nullptr };
   float* mWriteBuffer{ nullptr };
   bool mPlay{ false };
   Checkbox* mPlayCheckbox{ nullptr };
   bool mLoop{ false };
   Checkbox* mLoopCheckbox{ nullptr };
   float mClipStart{ 0 };
   FloatSlider* mClipStartSlider{ nullptr };
   float mClipEnd{ 1 };
   FloatSlider* mClipEndSlider{ nullptr };
   float mZoomStart{ 0 };
   FloatSlider* mZoomStartSlider{ nullptr };
   float mZoomEnd{ 1 };
   FloatSlider* mZoomEndSlider{ nullptr };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mNumBars{ 1 };
   IntSlider* mNumBarsSlider{ nullptr };
   ClickButton* mWriteButton{ nullptr };
   int mPlayhead{ 0 };
   float mTempo{ 120 };
   FloatSlider* mTempoSlider{ nullptr };
   int mStartOffset{ 0 };
   ClickButton* mCalcTempoButton{ nullptr };
   IntSlider* mStartOffsetSlider{ nullptr };
   ClickButton* mDoubleLengthButton{ nullptr };
   ClickButton* mHalveLengthButton{ nullptr };
   BiquadFilterEffect mBiquad[PRODUCER_NUM_BIQUADS];
   std::list<int> mSkipMeasures;
   ClickButton* mRestartButton{ nullptr };
};

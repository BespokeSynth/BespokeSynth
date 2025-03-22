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
//  SampleFinder.h
//  modularSynth
//
//  Created by Ryan Challinor on 6/18/13.
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
#include "SampleDrawer.h"

class Sample;

class SampleFinder : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   SampleFinder();
   ~SampleFinder();
   static IDrawableModule* Create() { return new SampleFinder(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
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

   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateSample();
   void DoWrite();
   void UpdateZoomExtents();
   float GetSpeed();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   Sample* mSample{ nullptr };

   float mVolume{ .6 };
   FloatSlider* mVolumeSlider{ nullptr };
   float* mWriteBuffer{ nullptr };
   bool mPlay{ false };
   Checkbox* mPlayCheckbox{ nullptr };
   bool mLoop{ true };
   Checkbox* mLoopCheckbox{ nullptr };
   int mMeasureEarly{ 0 };
   bool mEditMode{ true };
   Checkbox* mEditCheckbox{ nullptr };
   int mClipStart{ 0 };
   IntSlider* mClipStartSlider{ nullptr };
   int mClipEnd{ 1 };
   IntSlider* mClipEndSlider{ nullptr };
   float mZoomStart{ 0 };
   float mZoomEnd{ 1 };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mNumBars{ 1 };
   IntSlider* mNumBarsSlider{ nullptr };
   ClickButton* mWriteButton{ nullptr };
   double mPlayhead{ 0 };
   bool mWantWrite{ false };
   ClickButton* mDoubleLengthButton{ nullptr };
   ClickButton* mHalveLengthButton{ nullptr };
   SampleDrawer mSampleDrawer;
   bool mReverse{ false };
   Checkbox* mReverseCheckbox{ nullptr };
};

/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  TapeLooper.h
//
//  Created by Ryan Challinor on 5/27/26.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "RollingBuffer.h"
#include "RadioButton.h"
#include "IInputRecordable.h"
#include "SwitchAndRamp.h"
#include "ClickButton.h"

class TapeLooper : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IRadioButtonListener, public IIntSliderListener, public IInputRecordable, public IButtonListener
{
private:
   enum class TapeLooperState
   {
      Capture,
      Stop,
      Loop
   };

public:
   TapeLooper();
   virtual ~TapeLooper();
   static IDrawableModule* Create() { return new TapeLooper(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   bool IsResizable() const override { return true; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IInputRecordable
   void SetRecording(bool record) override;
   bool IsRecording() const override { return mRecording; }
   void ClearRecording() override { mState = TapeLooperState::Capture; }
   void CancelRecording() override { mState = TapeLooperState::Capture; }
   bool IsInRetroactiveRecorderMode() const override { return true; }
   void DoRetroactiveRecord(int numBars) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;

   int GetPlaybackSamplesAgo(double time) const;
   void StartLoop(int numBars);

   TapeLooperState mState{ TapeLooperState::Capture };
   RadioButton* mStateSelector{ nullptr };
   RollingBuffer mRecordBuffer;
   FloatSlider* mDisplayLengthSecondsSlider{ nullptr };
   float mDisplayLengthSeconds{ 20.0f };
   IntSlider* mLoopLengthBeatsSlider{ nullptr };
   int mLoopLengthBeats{ 4 };
   FloatSlider* mLoopBeatsAgoSlider{ nullptr };
   float mLoopBeatsAgo{ 0.0f };
   FloatSlider* mDownbeatOffsetBeatsSlider{ nullptr };
   float mDownbeatOffsetBeats{ 0.0f };
   Checkbox* mRecordCheckbox{ nullptr };
   bool mRecording{ false };
   ClickButton* mLoop1BarButton{ nullptr };
   ClickButton* mLoop2BarsButton{ nullptr };
   ClickButton* mLoop4BarsButton{ nullptr };
   ClickButton* mLoop8BarsButton{ nullptr };
   ClickButton* mLoop16BarsButton{ nullptr };
   float mLatencyFixMs{ 0.0f };
   FloatSlider* mLatencyFixMsSlider{ nullptr };
   bool mPassthrough{ true };
   Checkbox* mPassthroughCheckbox{ nullptr };
   Checkbox* mFirstLoopCheckbox{ nullptr };
   bool mFirstLoop{ false };

   double mLastCaptureMeasureTime{ 0.0 };
   double mStartRecordingMeasureTime{ -1 };
   double mStartRecordingTimeMs{ -1 };
   double mFirstLoopAudioStartMs{ -1 };
   int mLastPlayedSamplesAgo{ 0 };
   SwitchAndRamp mLoopWrapSmoother;
};

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
//  Tracker.h
//  Bespoke
//
//  A simple tracker / sample step-sequencer. The playhead advances one step per
//  transport tick (rate selectable: 4n / 8n / 8nt / 16n / ...), wrapping at the
//  chosen step count (1 / 4 / 8 / 16). Each step holds:
//    - a Sample you drag onto its row (empty = silent step)
//    - a volume + decay envelope (how the hit fades)
//    - a pitch in semitones (plays the sample faster/slower)
//    - a repeat count (retriggers the sample N times across the step - a roll)
//  It plays the samples itself (audio out), modeled on how DrumPlayer plays its
//  hits, so it plugs straight into gain / effects like any synth.
//

#pragma once

#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Sample.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "ChannelBuffer.h"
#include <array>
#include <vector>

class ofxJSONElement;

class Tracker : public IAudioSource, public IDrawableModule, public ITimeListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener
{
public:
   Tracker();
   ~Tracker();
   static IDrawableModule* Create() { return new Tracker(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IDrawableModule
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void UpdateTransportListener();
   void UpdateDimensions();
   void TriggerStepHit(int step, double time);
   void RandomizeSteps();
   void RandomizeStep(int step); //randomize a single step's vol/decay/pitch/repeat
   void ClearStep(int step); //delete/reset the sample loaded in a step
   void SwitchStepSample(int step, int dir); //load the next/closest sample from the scanned library

   static const int kMaxSteps = 32;
   static const int kMaxVoicesPerStep = 6;

   struct Voice
   {
      bool mActive{ false };
      double mOffset{ 0 }; //playback position in source samples
      double mEnvTimeMs{ 0 };
   };

   struct StepData
   {
      Sample mSample;
      bool mHasSample{ false };
      std::string mPath; //file path of the loaded sample (used to find the next one in the library)
      float mVol{ 1.0f };
      float mDecayMs{ 250.0f };
      float mPitch{ 0.0f }; //semitones
      int mRepeat{ 1 };
      std::array<Voice, kMaxVoicesPerStep> mVoices;

      FloatSlider* mVolSlider{ nullptr };
      FloatSlider* mDecaySlider{ nullptr };
      FloatSlider* mPitchSlider{ nullptr };
      IntSlider* mRepeatSlider{ nullptr };
      ClickButton* mRandButton{ nullptr }; //per-step randomize
      ClickButton* mClearButton{ nullptr }; //per-step delete/reset sample
      ClickButton* mNextButton{ nullptr }; //per-step switch to next sample in the library
   };

   struct ScheduledHit
   {
      int mStep{ -1 };
      double mTime{ 0 };
   };

   std::array<StepData, kMaxSteps> mSteps;
   std::vector<ScheduledHit> mScheduledHits;

   int mNumSteps{ 8 };
   IntSlider* mNumStepsSlider{ nullptr };
   NoteInterval mInterval{ kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   ClickButton* mRandomizeButton{ nullptr };
   float mGlobalVolume{ 1.0f }; //master volume for the whole tracker
   FloatSlider* mGlobalVolumeSlider{ nullptr };
   int mCurStep{ -1 };

   ChannelBuffer mWriteBuffer;
   float mWidth{ 520 };
   float mHeight{ 220 };
};

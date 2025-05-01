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
//  SamplerGrid.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/12/14.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "MidiDevice.h"
#include "Ramp.h"
#include "GridController.h"
#include "UIGrid.h"

class ofxJSONElement;

#define MAX_SAMPLER_GRID_LENGTH 5 * 44100 //just 5-ish seconds of audio. don't need to adjust for actual sample rate.
#define SAMPLE_RAMP_MS 3

class SamplerGrid : public IAudioProcessor, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IGridControllerListener, public UIGridListener, public INoteReceiver
{
public:
   SamplerGrid();
   ~SamplerGrid();
   static IDrawableModule* Create() { return new SamplerGrid(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void Poll() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;

   void InitGrid();
   void UpdateLights();

   int GridToIdx(int x, int y) { return x + y * mCols; }

   Checkbox* mClearCheckbox{ nullptr };
   bool mClear{ false };

   struct GridSample
   {
      float mSampleData[MAX_SAMPLER_GRID_LENGTH]{};
      int mPlayhead{ 0 };
      bool mHasSample{ false };
      bool mRecordingArmed{ false };
      int mSampleLength{ 0 };
      Ramp mRamp;
      int mSampleStart{ 0 };
      int mSampleEnd{ 0 };
   };

   void SetEditSample(GridSample* sample);

   GridSample* mGridSamples{ nullptr };
   int mRecordingSample{ -1 };

   bool mPassthrough{ true };
   Checkbox* mPassthroughCheckbox{ nullptr };

   float mVolume{ 1 };
   FloatSlider* mVolumeSlider{ nullptr };
   bool mEditMode{ false };
   Checkbox* mEditCheckbox{ nullptr };
   bool mDuplicate{ false };
   Checkbox* mDuplicateCheckbox{ nullptr };

   GridControlTarget* mGridControlTarget{ nullptr };
   int mCols{ 8 };
   int mRows{ 8 };
   bool mLastColumnIsGroup{ true }; //@TODO(Noxy): Not used?

   int mEditSampleX{ 2 };
   int mEditSampleY{ 95 };
   float mEditSampleWidth{ 395 };
   float mEditSampleHeight{ 200 };
   GridSample* mEditSample{ nullptr };
   IntSlider* mEditStartSlider{ nullptr };
   IntSlider* mEditEndSlider{ nullptr };
   int mDummyInt{ 0 };

   UIGrid* mGrid{ nullptr };
};

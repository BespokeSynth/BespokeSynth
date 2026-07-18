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
//  MolderSampler.h
//  Bespoke
//
//  A unified sample-molding instrument (inspired by Bitwig's sampler) that plays one dropped
//  sample through a selectable engine:
//    - Classic  : play/repitch by note, loop mode off / loop / ping-pong
//    - Slice    : auto-slice the sample (equal divisions OR detected onsets); each incoming note
//                 plays a slice, mapped up the keyboard from a base note
//    - Repitch  : repitch the whole sample to the played note, optionally snapped to the global scale
//    - Granular : grainy playback - scrub position, grain size, spread, density, freeze
//    - Spectral : FFT-freeze a window of the sample into a tone, with spectral tilt + harmonic boost
//  Built on Bespoke's own Sample / FFT / Scale facilities (all open source).
//

#pragma once

#include "IAudioProcessor.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Sample.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "ADSR.h"
#include "ADSRDisplay.h"
#include "ChannelBuffer.h"
#include <array>
#include <vector>

class MolderSampler : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener
{
public:
   MolderSampler();
   ~MolderSampler();
   static IDrawableModule* Create() { return new MolderSampler(); }
   static bool AcceptsAudio() { return true; } //feed an audio channel in to record it as the sample
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDrawableModule
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   void OnClicked(float x, float y, bool right) override; //click waveform to add/remove slice markers

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void Poll() override; //drives the draggable region markers while the mouse is held

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void RebuildSlices();
   void RebuildSpectral();
   void FinalizeRecording(); //turn the captured audio into the active sample
   int SourceLen() const;

   enum Mode
   {
      kMode_Classic = 0,
      kMode_Slice,
      kMode_Repitch,
      kMode_Granular,
      kMode_Spectral
   };

   Sample mSample;
   bool mHasSample{ false };

   //audio recording: arm, feed a channel in, capture to mRecordBuf, then bake into mSample
   bool mRecording{ false };
   bool mPendingFinalize{ false }; //audio thread requests, UI thread (Poll) bakes the sample
   Checkbox* mRecordCheckbox{ nullptr };
   std::vector<float> mRecordBuf; //pre-allocated on the UI thread; the audio thread only writes by index (RT-safe)
   size_t mRecordLen{ 0 };
   float mInputLevel{ 0 }; //smoothed input meter for feedback while armed

   int mMode{ kMode_Classic };
   DropdownList* mModeSelector{ nullptr };
   int mRootPitch{ 60 };
   IntSlider* mRootSlider{ nullptr };
   float mGain{ 1.0f };
   FloatSlider* mGainSlider{ nullptr };
   float mSpeed{ 1.0f };
   FloatSlider* mSpeedSlider{ nullptr };

   //GLOBAL play region + mode (honored by every engine): the sample only plays between the two
   //markers; mode = one-shot / loop / ping-pong / reverse
   float mRegionStart{ 0.0f }; //0-1 fraction of the sample
   float mRegionEnd{ 1.0f };
   int mRegionMode{ 1 }; //0 one-shot, 1 loop, 2 ping-pong, 3 reverse
   DropdownList* mRegionModeSelector{ nullptr };
   //marker drag state
   int mDragMarker{ 0 }; //0 none, 1 start, 2 end
   float mDragOffsetX{ 0 };
   //double-click detection for removing manual slices
   double mLastClickMs{ -9999 };
   float mLastClickLocalX{ -1 };

   //slice
   int mSliceMode{ 0 }; //0 divisions, 1 onsets, 2 manual
   DropdownList* mSliceModeSelector{ nullptr };
   int mNumSlices{ 8 };
   IntSlider* mNumSlicesSlider{ nullptr };
   float mOnsetSensitivity{ 0.5f };
   FloatSlider* mOnsetSensitivitySlider{ nullptr };
   int mBaseNote{ 60 };
   ClickButton* mResliceButton{ nullptr };
   ClickButton* mSlice4{ nullptr };
   ClickButton* mSlice8{ nullptr };
   ClickButton* mSlice16{ nullptr };
   ClickButton* mSlice32{ nullptr };
   std::vector<int> mSliceStarts; //boundaries in source samples, size = actual slices + 1

   //repitch
   bool mSnapScale{ false };
   Checkbox* mSnapScaleCheckbox{ nullptr };

   //granular
   float mGrainPos{ 0.0f }; //0-1 scrub through sample
   float mGrainSizeMs{ 90.0f };
   float mGrainSpread{ 0.05f };
   float mGrainDensity{ 0.6f };
   bool mFreeze{ false };
   FloatSlider* mGrainPosSlider{ nullptr };
   FloatSlider* mGrainSizeSlider{ nullptr };
   FloatSlider* mGrainSpreadSlider{ nullptr };
   FloatSlider* mGrainDensitySlider{ nullptr };
   Checkbox* mFreezeCheckbox{ nullptr };

   //spectral
   float mSpecPos{ 0.0f };
   float mSpecTilt{ 0.0f }; //-1 darker .. +1 brighter
   float mSpecHarmonics{ 0.0f }; //0..1 boost of upper partials
   FloatSlider* mSpecPosSlider{ nullptr };
   FloatSlider* mSpecTiltSlider{ nullptr };
   FloatSlider* mSpecHarmonicsSlider{ nullptr };
   static const int kSpecSize = 1024;
   std::vector<float> mSpecTable; //frozen single-window tone (kSpecSize samples)
   bool mSpecDirty{ true };

   //amp envelope (shared)
   ::ADSR mAdsr;
   ADSRDisplay* mADSRDisplay{ nullptr };

   struct Voice
   {
      bool mActive{ false };
      int mPitch{ 60 };
      float mVel{ 1.0f };
      double mStartTime{ 0 };
      ::ADSR mAdsr;

      //classic / slice / repitch playhead (source-sample position)
      double mPos{ 0 };
      int mDir{ 1 }; //ping-pong direction
      double mRegionStart{ 0 }; //slice / loop region
      double mRegionEnd{ 0 };
      bool mOneShotDone{ false };

      //granular scrub state
      double mScrub{ 0 }; //source position the grains read around
      double mGrainPhase{ 0 }; //0..1 through the current grain
      double mGrainStart{ 0 }; //source pos this grain began at

      //spectral phase (reads mSpecTable)
      double mSpecPhase{ 0 };
   };
   static const int kNumVoices = 8;
   std::array<Voice, kNumVoices> mVoices;

   ChannelBuffer mWriteBuffer;
   float mWidth{ 470 };
   float mHeight{ 244 };
};

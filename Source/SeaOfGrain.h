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
//  SeaOfGrain.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteReceiver.h"
#include "Granulator.h"
#include "ADSR.h"

class Sample;

class SeaOfGrain : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   SeaOfGrain();
   ~SeaOfGrain();
   static IDrawableModule* Create() { return new SeaOfGrain(); }
   static bool AcceptsAudio() { return true; }
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
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   void Poll() override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;


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

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateSample();
   void UpdateDisplaySamples();
   void LoadFile();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   float GetSampleRateRatio() const;
   ChannelBuffer* GetSourceBuffer();
   float GetSourceStartSample();
   float GetSourceEndSample();
   float GetSourceBufferOffset();

   struct GrainMPEVoice
   {
      GrainMPEVoice();
      void Process(ChannelBuffer* output, int bufferSize);
      void Draw(float w, float h);

      float mPlay{ 0 };
      float mPitch{ 0 };
      ModulationChain* mPitchBend{ nullptr };
      ModulationChain* mPressure{ nullptr };
      ModulationChain* mModWheel{ nullptr };

      float mGain{ 0 };

      ::ADSR mADSR{ 100, 0, 1, 100 };
      Granulator mGranulator;
      SeaOfGrain* mOwner{ nullptr };
   };

   struct GrainManualVoice
   {
      GrainManualVoice();
      void Process(ChannelBuffer* output, int bufferSize);
      void Draw(float w, float h);

      float mGain{ 0 };
      float mPosition{ 0 };
      float mPan{ 0 };

      Granulator mGranulator;
      SeaOfGrain* mOwner{ nullptr };

      FloatSlider* mGainSlider{ nullptr };
      FloatSlider* mPositionSlider{ nullptr };
      FloatSlider* mOverlapSlider{ nullptr };
      FloatSlider* mSpeedSlider{ nullptr };
      FloatSlider* mLengthMsSlider{ nullptr };
      FloatSlider* mPosRandomizeSlider{ nullptr };
      FloatSlider* mSpeedRandomizeSlider{ nullptr };
      FloatSlider* mSpacingRandomizeSlider{ nullptr };
      Checkbox* mOctaveCheckbox{ nullptr };
      FloatSlider* mWidthSlider{ nullptr };
      FloatSlider* mPanSlider{ nullptr };
   };

   static const int kNumMPEVoices = 16;
   GrainMPEVoice mMPEVoices[kNumMPEVoices];
   static const int kNumManualVoices = 6;
   GrainManualVoice mManualVoices[kNumManualVoices];

   Sample* mSample{ nullptr };
   RollingBuffer mRecordBuffer;

   ClickButton* mLoadButton{ nullptr };
   bool mRecordInput{ false };
   Checkbox* mRecordInputCheckbox{ nullptr };
   bool mHasRecordedInput{ false };

   float mVolume{ .6 };
   FloatSlider* mVolumeSlider{ nullptr };
   bool mLoading{ false };
   FloatSlider* mDisplayOffsetSlider{ nullptr };
   float mDisplayOffset{ 0 };
   FloatSlider* mDisplayLengthSlider{ nullptr };
   float mDisplayLength{ 10 };
   int mDisplayStartSamples{ 0 };
   int mDisplayEndSamples{ 0 };
   DropdownList* mKeyboardBasePitchSelector{ nullptr };
   int mKeyboardBasePitch{ 36 };
   DropdownList* mKeyboardNumPitchesSelector{ nullptr };
   int mKeyboardNumPitches{ 24 };
};

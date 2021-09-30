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

#ifndef __Bespoke__SeaOfGrain__
#define __Bespoke__SeaOfGrain__

#include <iostream>
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
   
   std::string GetTitleLabel() override { return "sea of grain"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
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
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void UpdateSample();
   void UpdateDisplaySamples();
   void LoadFile();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;

   ChannelBuffer* GetSourceBuffer();
   float GetSourceStartSample();
   float GetSourceEndSample();
   float GetSourceBufferOffset();
   
   struct GrainMPEVoice
   {
      GrainMPEVoice();
      void Process(ChannelBuffer* output, int bufferSize);
      void Draw(float w, float h);
      
      float mPlay;
      float mPitch;
      ModulationChain* mPitchBend;
      ModulationChain* mPressure;
      ModulationChain* mModWheel;
      
      float mGain;
      
      ::ADSR mADSR;
      Granulator mGranulator;
      SeaOfGrain* mOwner;
   };
   
   struct GrainManualVoice
   {
      GrainManualVoice();
      void Process(ChannelBuffer* output, int bufferSize);
      void Draw(float w, float h);
      
      float mGain;
      float mPosition;
      float mPan;
      
      Granulator mGranulator;
      SeaOfGrain* mOwner;
      
      FloatSlider* mGainSlider;
      FloatSlider* mPositionSlider;
      FloatSlider* mOverlapSlider;
      FloatSlider* mSpeedSlider;
      FloatSlider* mLengthMsSlider;
      FloatSlider* mPosRandomizeSlider;
      FloatSlider* mSpeedRandomizeSlider;
      FloatSlider* mSpacingRandomizeSlider;
      Checkbox* mOctaveCheckbox;
      FloatSlider* mWidthSlider;
      FloatSlider* mPanSlider;
   };
   
   static const int kNumMPEVoices = 16;
   GrainMPEVoice mMPEVoices[kNumMPEVoices];
   static const int kNumManualVoices = 6;
   GrainManualVoice mManualVoices[kNumManualVoices];
   
   Sample* mSample;
   RollingBuffer mRecordBuffer;
   
   ClickButton* mLoadButton;
   bool mRecordInput;
   Checkbox* mRecordInputCheckbox;
   bool mHasRecordedInput;
   float mVolume;
   FloatSlider* mVolumeSlider;
   bool mLoading;
   FloatSlider* mDisplayOffsetSlider;
   float mDisplayOffset;
   FloatSlider* mDisplayLengthSlider;
   float mDisplayLength;
   int mDisplayStartSamples;
   int mDisplayEndSamples;
   DropdownList* mKeyboardBasePitchSelector;
   int mKeyboardBasePitch;
   DropdownList* mKeyboardNumPitchesSelector;
   int mKeyboardNumPitches;
};

#endif /* defined(__Bespoke__SeaOfGrain__) */


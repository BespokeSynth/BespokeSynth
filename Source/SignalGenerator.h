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
//  SignalGenerator.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "IPulseReceiver.h"

class ofxJSONElement;

class SignalGenerator : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IPulseReceiver
{
public:
   SignalGenerator();
   ~SignalGenerator();
   static IDrawableModule* Create() { return new SignalGenerator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetVol(double vol) { mVol = vol; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPulseReceiver
   void OnPulse(double time, double velocity, int flags) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& width, double& height) override;

   enum FreqMode
   {
      kFreqMode_Instant,
      kFreqMode_Root,
      kFreqMode_Ramp,
      kFreqMode_Slider
   };

   void SetType(OscillatorType type);
   void SetFreqMode(FreqMode mode);

   double mVol{ 0 };
   FloatSlider* mVolSlider{ nullptr };
   OscillatorType mOscType{ OscillatorType::kOsc_Sin };
   DropdownList* mOscSelector{ nullptr };
   double mPulseWidth{ .5 };
   FloatSlider* mPulseWidthSlider{ nullptr };
   double mShuffle{ 0 };
   FloatSlider* mShuffleSlider{ nullptr };
   int mMult{ 1 };
   DropdownList* mMultSelector{ nullptr };
   Oscillator::SyncMode mSyncMode{ Oscillator::SyncMode::None };
   DropdownList* mSyncModeSelector{ nullptr };
   double mSyncFreq{ 200 };
   FloatSlider* mSyncFreqSlider{ nullptr };
   double mSyncRatio{ 1 };
   FloatSlider* mSyncRatioSlider{ nullptr };
   double mDetune{ 0 };
   FloatSlider* mDetuneSlider{ nullptr };
   EnvOscillator mOsc{ OscillatorType::kOsc_Sin };
   double mFreq{ 220 };
   FloatSlider* mFreqSlider{ nullptr };
   double mPhase{ 0 };
   double mSyncPhase{ 0 };
   FreqMode mFreqMode{ FreqMode::kFreqMode_Instant };
   DropdownList* mFreqModeSelector{ nullptr };
   double mFreqSliderStart{ 220 };
   double mFreqSliderEnd{ 220 };
   double mFreqSliderAmount{ 0 };
   FloatSlider* mFreqSliderAmountSlider{ nullptr };
   Ramp mFreqRamp;
   double mFreqRampTime{ 200 };
   FloatSlider* mFreqRampTimeSlider{ nullptr };
   double mSoften{ 0 };
   FloatSlider* mSoftenSlider{ nullptr };
   double mPhaseOffset{ 0 };
   FloatSlider* mPhaseOffsetSlider{ nullptr };
   double mResetPhaseAtMs{ -9999 };

   float* mWriteBuffer{ nullptr };

   int mLoadRev{ -1 };
};

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
//  VinylTempoControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/18/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IModulator.h"
// clang-format off
extern "C"
{
#include "xwax/timecoder.h"
}
// clang-format on

class VinylProcessor
{
public:
   VinylProcessor(int sampleRate);
   ~VinylProcessor();

   void Process(float* left, float* right, int numSamples);

   float GetPitch() { return mPitch; }
   bool GetStopped() { return mHasSignal == false; } //@TODO(Noxy): There is no way for mHasSignal to go true so GetStopped() (which is used in other places) is always true.

private:
   int mSampleRate;

   float mPitch{ 0 };
   bool mHasSignal{ false };

   timecoder mTimecoder;
};

class VinylTempoControl : public IDrawableModule, public IAudioProcessor, public IModulator
{
public:
   VinylTempoControl();
   ~VinylTempoControl();
   static IDrawableModule* Create() { return new VinylTempoControl(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }
   bool ShouldSuppressAutomaticOutputCable() override { return true; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void CreateUIControls() override;

   void Process(double time) override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   bool CanStartVinylControl();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 90;
      height = 20;
   }

   bool mUseVinylControl{ false };
   Checkbox* mUseVinylControlCheckbox{ nullptr };
   float mReferencePitch{ 1 };
   VinylProcessor mVinylProcessor;
   //float* mModulationBuffer;
   float mSpeed{ 1 };
};

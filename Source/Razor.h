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
//  Razor.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/3/12.
//
//

#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "ADSR.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "ClickButton.h"

#define NUM_PARTIALS 320
#define VIZ_WIDTH 1000
#define RAZOR_HISTORY 100
#define NUM_BUMPS 3
#define NUM_AMP_SLIDERS 16

struct RazorBump
{
   float mFreq{ 100 };
   float mAmt{ 0 };
   float mDecay{ .005 };
};

class Razor : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IButtonListener
{
public:
   Razor();
   ~Razor();
   static IDrawableModule* Create() { return new Razor(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   float SinSample(float phase); //phase 0-512
   void CalcAmp();
   void DrawViz();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 1020;
      h = 420;
   }

   float mVol{ .05 };
   float mPhase{ 0 };
   ::ADSR mAdsr[NUM_PARTIALS]{};
   float mAmp[NUM_PARTIALS]{};
   float mPhases[NUM_PARTIALS]{};
   float mDetune[NUM_PARTIALS]{};

   int mPitch{ -1 };

   int mUseNumPartials{ NUM_PARTIALS };
   IntSlider* mNumPartialsSlider{ nullptr };
   RazorBump mBumps[NUM_BUMPS];
   FloatSlider* mBumpAmpSlider{ nullptr };
   FloatSlider* mBumpAmpAmtSlider{ nullptr };
   FloatSlider* mBumpAmpDecaySlider{ nullptr };
   FloatSlider* mBumpAmpSlider2{ nullptr };
   FloatSlider* mBumpAmpAmtSlider2{ nullptr };
   FloatSlider* mBumpAmpDecaySlider2{ nullptr };
   FloatSlider* mBumpAmpSlider3{ nullptr };
   FloatSlider* mBumpAmpAmtSlider3{ nullptr };
   FloatSlider* mBumpAmpDecaySlider3{ nullptr };
   FloatSlider* mASlider{ nullptr };
   FloatSlider* mDSlider{ nullptr };
   FloatSlider* mSSlider{ nullptr };
   FloatSlider* mRSlider{ nullptr };
   int mHarmonicSelector{ 1 };
   IntSlider* mHarmonicSelectorSlider{ nullptr };
   float mPowFalloff{ 1 };
   FloatSlider* mPowFalloffSlider{ nullptr };
   int mNegHarmonics{ 0 };
   IntSlider* mNegHarmonicsSlider{ nullptr };
   float mHarshnessCut{ 0 };
   FloatSlider* mHarshnessCutSlider{ nullptr };

   bool mManualControl{ false };
   Checkbox* mManualControlCheckbox{ nullptr };
   FloatSlider* mAmpSliders[NUM_AMP_SLIDERS]{ nullptr };
   FloatSlider* mDetuneSliders[NUM_AMP_SLIDERS]{ nullptr };
   ClickButton* mResetDetuneButton{ nullptr };

   float mA{ 1 };
   float mD{ 0 };
   float mS{ 1 };
   float mR{ 1 };

   ModulationChain* mPitchBend{ nullptr };
   ModulationChain* mModWheel{ nullptr };
   ModulationChain* mPressure{ nullptr };

   float mPeakHistory[RAZOR_HISTORY][VIZ_WIDTH + 1]{};
   int mHistoryPtr{ 0 };
};

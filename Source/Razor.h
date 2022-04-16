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

#ifndef __modularSynth__Razor__
#define __modularSynth__Razor__

#include <iostream>
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
   float mFreq;
   float mAmt;
   float mDecay;
};

class Razor : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IButtonListener
{
public:
   Razor();
   ~Razor();
   static IDrawableModule* Create() { return new Razor(); }


   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;


private:
   float SinSample(float phase); //phase 0-512
   void CalcAmp();
   void DrawViz();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 1020;
      h = 420;
   }

   float mVol;
   float mPhase;
   ::ADSR mAdsr[NUM_PARTIALS];
   float mAmp[NUM_PARTIALS];
   float mPhases[NUM_PARTIALS];
   float mDetune[NUM_PARTIALS];

   int mPitch;

   int mUseNumPartials;
   IntSlider* mNumPartialsSlider;
   RazorBump mBumps[NUM_BUMPS];
   FloatSlider* mBumpAmpSlider;
   FloatSlider* mBumpAmpAmtSlider;
   FloatSlider* mBumpAmpDecaySlider;
   FloatSlider* mBumpAmpSlider2;
   FloatSlider* mBumpAmpAmtSlider2;
   FloatSlider* mBumpAmpDecaySlider2;
   FloatSlider* mBumpAmpSlider3;
   FloatSlider* mBumpAmpAmtSlider3;
   FloatSlider* mBumpAmpDecaySlider3;
   FloatSlider* mASlider;
   FloatSlider* mDSlider;
   FloatSlider* mSSlider;
   FloatSlider* mRSlider;
   int mHarmonicSelector;
   IntSlider* mHarmonicSelectorSlider;
   float mPowFalloff;
   FloatSlider* mPowFalloffSlider;
   int mNegHarmonics;
   IntSlider* mNegHarmonicsSlider;
   float mHarshnessCut;
   FloatSlider* mHarshnessCutSlider;

   bool mManualControl;
   Checkbox* mManualControlCheckbox;
   FloatSlider* mAmpSliders[NUM_AMP_SLIDERS];
   FloatSlider* mDetuneSliders[NUM_AMP_SLIDERS];
   ClickButton* mResetDetuneButton;

   float mA;
   float mD;
   float mS;
   float mR;

   ModulationChain* mPitchBend;
   ModulationChain* mModWheel;
   ModulationChain* mPressure;

   float mPeakHistory[RAZOR_HISTORY][VIZ_WIDTH + 1];
   int mHistoryPtr;
};

#endif /* defined(__modularSynth__Razor__) */

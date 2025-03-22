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
//  Autotalent.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/27/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "Slider.h"
#include "Checkbox.h"
#include "IDrawableModule.h"
#include "RadioButton.h"
#include "ClickButton.h"
#include "INoteReceiver.h"

class FFT;

class Autotalent : public IAudioProcessor, public IIntSliderListener, public IFloatSliderListener, public IDrawableModule, public IRadioButtonListener, public IButtonListener, public INoteReceiver
{
public:
   Autotalent();
   ~Autotalent();
   static IDrawableModule* Create() { return new Autotalent(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override {}
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateShiftSlider();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 260;
      height = 360;
   }

   float* mWorkingBuffer;

   RadioButton* mASelector{ nullptr };
   RadioButton* mBbSelector{ nullptr };
   RadioButton* mBSelector{ nullptr };
   RadioButton* mCSelector{ nullptr };
   RadioButton* mDbSelector{ nullptr };
   RadioButton* mDSelector{ nullptr };
   RadioButton* mEbSelector{ nullptr };
   RadioButton* mESelector{ nullptr };
   RadioButton* mFSelector{ nullptr };
   RadioButton* mGbSelector{ nullptr };
   RadioButton* mGSelector{ nullptr };
   RadioButton* mAbSelector{ nullptr };

   FloatSlider* mAmountSlider{ nullptr };
   FloatSlider* mSmoothSlider{ nullptr };
   IntSlider* mShiftSlider{ nullptr };
   IntSlider* mScwarpSlider{ nullptr };
   FloatSlider* mLfoampSlider{ nullptr };
   FloatSlider* mLforateSlider{ nullptr };
   IntSlider* mLfoshapeSlider{ nullptr };
   FloatSlider* mLfosymmSlider{ nullptr };
   Checkbox* mLfoquantCheckbox{ nullptr };
   Checkbox* mFcorrCheckbox{ nullptr };
   FloatSlider* mFwarpSlider{ nullptr };
   FloatSlider* mMixSlider{ nullptr };

   ClickButton* mSetFromScaleButton{ nullptr };

   ////////////////////////////////////////
   //ported
   float mTune{ 440 };
   float mFixed{ 0 };
   float mPull{ 0 };
   int mA{ 0 };
   int mBb{ 0 };
   int mB{ 0 };
   int mC{ 0 };
   int mDb{ 0 };
   int mD{ 0 };
   int mEb{ 0 };
   int mE{ 0 };
   int mF{ 0 };
   int mGb{ 0 };
   int mG{ 0 };
   int mAb{ 0 };
   float mAmount{ 1 };
   float mSmooth{ 0 };
   int mShift{ 0 };
   int mScwarp{ 0 };
   float mLfoamp{ 0 };
   float mLforate{ 0 };
   int mLfoshape{ 0 };
   float mLfosymm{ 0 };
   bool mLfoquant{ false };
   bool mFcorr{ false };
   float mFwarp{ 0 };
   float mMix{ 1 };
   float mPitch{ 0 };
   float mConfidence{ 0 };
   float mLatency{ 0 };
   ::FFT* mFFT;

   unsigned long mfs; // Sample rate

   unsigned long mcbsize; // size of circular buffer
   unsigned long mcorrsize; // cbsize/2 + 1
   unsigned long mcbiwr;
   unsigned long mcbord;
   float* mcbi; // circular input buffer
   float* mcbf; // circular formant correction buffer
   float* mcbo; // circular output buffer

   float* mcbwindow; // hann of length N/2, zeros for the rest
   float* macwinv; // inverse of autocorrelation of window
   float* mhannwindow; // length-N hann
   int mnoverlap;

   float* mffttime;
   float* mfftfreqre;
   float* mfftfreqim;

   // VARIABLES FOR LOW-RATE SECTION
   float maref{ 440 }; // A tuning reference (Hz)
   float minpitch{ 0 }; // Input pitch (semitones)
   float mconf{ 0 }; // Confidence of pitch period estimate (between 0 and 1)
   float moutpitch{ 0 }; // Output pitch (semitones)
   float mvthresh; // Voiced speech threshold

   float mpmax; // Maximum allowable pitch period (seconds)
   float mpmin; // Minimum allowable pitch period (seconds)
   unsigned long mnmax; // Maximum period index for pitch prd est
   unsigned long mnmin; // Minimum period index for pitch prd est

   float mlrshift; // Shift prescribed by low-rate section
   int mptarget; // Pitch target, between 0 and 11
   float msptarget; // Smoothed pitch target

   float mlfophase;

   // VARIABLES FOR PITCH SHIFTER
   float mphprdd; // default (unvoiced) phase period
   double minphinc; // input phase increment
   double moutphinc{ 0 }; // input phase increment
   double mphincfact; // factor determining output phase increment
   double mphasein;
   double mphaseout;
   float* mfrag; // windowed fragment of speech
   unsigned long mfragsize; // size of fragment in samples

   // VARIABLES FOR FORMANT CORRECTOR
   int mford;
   float mfalph;
   float mflamb;
   float* mfk;
   float* mfb;
   float* mfc;
   float* mfrb;
   float* mfrc;
   float* mfsig;
   float* mfsmooth;
   float mfhp;
   float mflp;
   float mflpa;
   float** mfbuff;
   float* mftvec;
   float mfmute;
   float mfmutealph;
};

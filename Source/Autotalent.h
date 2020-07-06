//
//  Autotalent.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/27/13.
//
//

#ifndef __modularSynth__Autotalent__
#define __modularSynth__Autotalent__

#include <iostream>
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
   
   string GetTitleLabel() override;
   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox) override {}
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override {}
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void UpdateShiftSlider();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=260; height=360; }
   bool Enabled() const override { return mEnabled; }

   float* mWorkingBuffer;

   RadioButton* mASelector;
   RadioButton* mBbSelector;
   RadioButton* mBSelector;
   RadioButton* mCSelector;
   RadioButton* mDbSelector;
   RadioButton* mDSelector;
   RadioButton* mEbSelector;
   RadioButton* mESelector;
   RadioButton* mFSelector;
   RadioButton* mGbSelector;
   RadioButton* mGSelector;
   RadioButton* mAbSelector;

   FloatSlider* mAmountSlider;
   FloatSlider* mSmoothSlider;
   IntSlider* mShiftSlider;
   IntSlider* mScwarpSlider;
   FloatSlider* mLfoampSlider;
   FloatSlider* mLforateSlider;
   IntSlider* mLfoshapeSlider;
   FloatSlider* mLfosymmSlider;
   Checkbox* mLfoquantCheckbox;
   Checkbox* mFcorrCheckbox;
   FloatSlider* mFwarpSlider;
   FloatSlider* mMixSlider;
   
   ClickButton* mSetFromScaleButton;

////////////////////////////////////////
   //ported
   float mTune;
   float mFixed;
   float mPull;
   int mA;
   int mBb;
   int mB;
   int mC;
   int mDb;
   int mD;
   int mEb;
   int mE;
   int mF;
   int mGb;
   int mG;
   int mAb;
   float mAmount;
   float mSmooth;
   int mShift;
   int mScwarp;
   float mLfoamp;
   float mLforate;
   int mLfoshape;
   float mLfosymm;
   bool mLfoquant;
   bool mFcorr;
   float mFwarp;
   float mMix;
   float mPitch;
   float mConfidence;
   float mInputBuffer1;
   float mOutputBuffer1;
   float mLatency;
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
   float maref; // A tuning reference (Hz)
   float minpitch; // Input pitch (semitones)
   float mconf; // Confidence of pitch period estimate (between 0 and 1)
   float moutpitch; // Output pitch (semitones)
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
   double moutphinc; // input phase increment
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

#endif /* defined(__modularSynth__Autotalent__) */


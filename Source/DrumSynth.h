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
//  DrumSynth.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/5/14.
//
//

#pragma once

#include <iostream>
#include "IAudioSource.h"
#include "Sample.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "Transport.h"
#include "EnvOscillator.h"
#include "ADSR.h"
#include "RadioButton.h"
#include "PeakTracker.h"
#include "BiquadFilter.h"
#include "TextEntry.h"
#include "PatchCableSource.h"

class MidiController;
class ADSRDisplay;

#define DRUMSYNTH_PAD_WIDTH 180
#define DRUMSYNTH_PAD_HEIGHT 236
#define DRUMSYNTH_PADS_HORIZONTAL 4
#define DRUMSYNTH_PADS_VERTICAL 2

class DrumSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public IRadioButtonListener, public ITextEntryListener
{
public:
   DrumSynth();
   ~DrumSynth();
   static IDrawableModule* Create() { return new DrumSynth(); }


   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return mUseIndividualOuts ? (int)mHits.size() + 1 : 1; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   struct DrumSynthHitSerialData
   {
      DrumSynthHitSerialData();

      EnvOscillator mTone;
      EnvOscillator mNoise;
      ::ADSR mFreqAdsr;
      ::ADSR mFilterAdsr;
      float mFreqMax;
      float mFreqMin;
      float mVol;
      float mVolNoise;
      float mCutoffMax;
      float mCutoffMin;
      float mQ;
   };

   struct IndividualOutput;

   class DrumSynthHit
   {
   public:
      DrumSynthHit(DrumSynth* parent, int index, int x, int y);
      ~DrumSynthHit();

      void CreateUIControls();
      void Play(double time, float velocity);
      void Process(double time, float* out, int bufferSize, int oversampling, double sampleRate, double sampleIncrementMs);
      float Level() { return mLevel.GetPeak(); }
      void Draw();

      DrumSynthHitSerialData mData;
      float mPhase;
      ADSRDisplay* mToneAdsrDisplay;
      ADSRDisplay* mFreqAdsrDisplay;
      ADSRDisplay* mNoiseAdsrDisplay;
      ADSRDisplay* mFilterAdsrDisplay;
      FloatSlider* mVolSlider;
      FloatSlider* mFreqMaxSlider;
      FloatSlider* mFreqMinSlider;
      RadioButton* mToneType;
      double mStartTime;
      PeakTracker mLevel;
      FloatSlider* mVolNoiseSlider;
      FloatSlider* mFilterCutoffMaxSlider;
      FloatSlider* mFilterCutoffMinSlider;
      FloatSlider* mFilterQSlider;
      DrumSynth* mParent;
      int mIndex;
      int mX;
      int mY;
      BiquadFilter mFilter;

      IndividualOutput* mIndividualOutput;
   };

   struct IndividualOutput
   {
      IndividualOutput(DrumSynthHit* owner)
         : mHit(owner)
         , mVizBuffer(nullptr)
         , mPatchCableSource(nullptr)
      {
         mVizBuffer = new RollingBuffer(VIZ_BUFFER_SECONDS*gSampleRate);
         mPatchCableSource = new PatchCableSource(owner->mParent, kConnectionType_Audio);

         mPatchCableSource->SetOverrideVizBuffer(mVizBuffer);
         mPatchCableSource->SetManualSide(PatchCableSource::Side::kRight);
         owner->mParent->AddPatchCableSource(mPatchCableSource);

         mPatchCableSource->SetManualPosition(mHit->mX + 30, mHit->mY + 8);
      }
      ~IndividualOutput()
      {
         delete mVizBuffer;
      }
      DrumSynthHit* mHit;
      RollingBuffer* mVizBuffer;
      PatchCableSource* mPatchCableSource;
   };

   int GetAssociatedSampleIndex(int x, int y);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;

   std::array<DrumSynthHit*, DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL> mHits;
   std::array<float, DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL> mVelocity;

   float mVolume;
   FloatSlider* mVolSlider;
   bool mUseIndividualOuts;
   bool mMonoOutput;
   int mOversampling{ 1 };
   DropdownList* mOversamplingSelector;
};


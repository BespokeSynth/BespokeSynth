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

#include "IAudioSource.h"
#include "Sample.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
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
#define DRUMSYNTH_NO_CUTOFF 10000

class DrumSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public IRadioButtonListener, public ITextEntryListener
{
public:
   DrumSynth();
   ~DrumSynth();
   static IDrawableModule* Create() { return new DrumSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return mUseIndividualOuts ? (int)mHits.size() + 1 : 1; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   struct DrumSynthHitSerialData
   {
      DrumSynthHitSerialData();

      EnvOscillator mTone{ OscillatorType::kOsc_Sin };
      EnvOscillator mNoise{ OscillatorType::kOsc_Random };
      ::ADSR mFreqAdsr;
      ::ADSR mFilterAdsr;
      double mFreqMax{ 150 };
      double mFreqMin{ 10 };
      double mVol{ 0 };
      double mVolNoise{ 0 };
      double mCutoffMax{ DRUMSYNTH_NO_CUTOFF };
      double mCutoffMin{ 10 };
      double mQ{ sqrt(2) / 2 };
   };

   struct IndividualOutput;

   class DrumSynthHit
   {
   public:
      DrumSynthHit(DrumSynth* parent, int index, int x, int y);
      ~DrumSynthHit();

      void CreateUIControls();
      void Play(double time, double velocity);
      void Process(double time, float* out, int bufferSize, int oversampling, double sampleRate, double sampleIncrementMs);
      double Level() { return mLevel.GetPeak(); }
      void Draw();

      DrumSynthHitSerialData mData;
      double mPhase{ 0 };
      ADSRDisplay* mToneAdsrDisplay{ nullptr };
      ADSRDisplay* mFreqAdsrDisplay{ nullptr };
      ADSRDisplay* mNoiseAdsrDisplay{ nullptr };
      ADSRDisplay* mFilterAdsrDisplay{ nullptr };
      FloatSlider* mVolSlider{ nullptr };
      FloatSlider* mFreqMaxSlider{ nullptr };
      FloatSlider* mFreqMinSlider{ nullptr };
      RadioButton* mToneType{ nullptr };
      PeakTracker mLevel;
      FloatSlider* mVolNoiseSlider{ nullptr };
      FloatSlider* mFilterCutoffMaxSlider{ nullptr };
      FloatSlider* mFilterCutoffMinSlider{ nullptr };
      FloatSlider* mFilterQSlider{ nullptr };
      DrumSynth* mParent;
      int mIndex{ 0 };
      int mX{ 0 };
      int mY{ 0 };
      BiquadFilter mFilter;

      IndividualOutput* mIndividualOutput{ nullptr };
   };

   struct IndividualOutput
   {
      IndividualOutput(DrumSynthHit* owner)
      : mHit(owner)
      {
         mVizBuffer = new RollingBuffer(VIZ_BUFFER_SECONDS * gSampleRate);
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
      DrumSynthHit* mHit{ nullptr };
      RollingBuffer* mVizBuffer{ nullptr };
      PatchCableSource* mPatchCableSource{ nullptr };
   };

   int GetAssociatedSampleIndex(int x, int y);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& width, double& height) override;
   void OnClicked(double x, double y, bool right) override;

   std::array<DrumSynthHit*, DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL> mHits;
   std::array<float, DRUMSYNTH_PADS_HORIZONTAL * DRUMSYNTH_PADS_VERTICAL> mVelocity{};

   double mVolume{ 1 };
   FloatSlider* mVolSlider{ nullptr };
   bool mUseIndividualOuts{ false };
   bool mMonoOutput{ false };
   int mOversampling{ 1 };
};

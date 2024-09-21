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
//  EuclideanSequencer.h
//  Bespoke
//
//  Created by Jack van Klaren on Mar 17 2024.
//  Based on CircleSequencer by Ryan Challinor
//
//

#pragma once

#include "Transport.h"
#include "UIGrid.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Scale.h"


class EuclideanSequencer;

#define EUCLIDEAN_SEQUENCER_MAX_STEPS 32
#define EUCLIDEAN_ROTATION_MIN -16
#define EUCLIDEAN_ROTATION_MAX 16
#define EUCLIDEAN_INITIALSTATE_MAX 4

class EuclideanSequencerRing
{
public:
   EuclideanSequencerRing(EuclideanSequencer* owner, int index);
   void Draw();
   void OnClicked(float x, float y, bool right);
   void SetSteps(int steps);
   int GetSteps();
   void SetOnsets(int onsets);
   void SetRotation(int rotation);
   void SetOffset(float offset);
   void SetPitch(int pitch);
   int GetPitch();
   void MouseReleased();
   void MouseMoved(float x, float y);
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time);
   void CreateUIControls();
   void InitialState(int state);
   void Clear();
   void OnTransportAdvanced(float amount);
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   float GetRadius() { return 90 - mIndex * 15; }
   int GetStepIndex(int x, int y, float& radiusOut);
   EuclideanSequencer* mOwner{ nullptr };
   int mIndex{ 0 };
   std::array<float, EUCLIDEAN_SEQUENCER_MAX_STEPS> mSteps{};
   float mLength{ 4 };
   FloatSlider* mLengthSlider{ nullptr };
   float mOnset{ 4 };
   FloatSlider* mOnsetSlider{ nullptr };
   float mRotation{ 0 };
   FloatSlider* mRotationSlider{ nullptr };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mPitch{ 0 };
   TextEntry* mNoteSelector{ nullptr };
   AdditionalNoteCable* mDestinationCable{ nullptr };


   int mCurrentlyClickedStepIdx{ -1 };
   int mHighlightStepIdx{ -1 };
   float mLastMouseRadius{ -1 };

   std::string GetEuclideanRhythm(int pulses, int steps, int rotation);
};

class EuclideanSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITextEntryListener, public IButtonListener
{
public:
   EuclideanSequencer();
   ~EuclideanSequencer();
   static IDrawableModule* Create() { return new EuclideanSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool on) override { mEnabled = on; }

   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

   bool PlayShortNotes() const { return mPlayShortNotes; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   float mWidth{ 785 }; // was 650 = random buttons visible
   float mHeight{ 200 };
   const float mWidthMin = 590;
   const float mWidthMax = 785;
   void OnClicked(float x, float y, bool right) override;

   // ModuleSaveData
   bool mPlayShortNotes{ false };

   ClickButton* mRandomizeButton{ nullptr };
   ClickButton* mRndLengthButton{ nullptr };
   ClickButton* mRndOnsetsButton{ nullptr };
   ClickButton* mRndRotationButton{ nullptr };
   ClickButton* mRndOffsetButton{ nullptr };
   ClickButton* mRndNoteButton{ nullptr };
   ClickButton* mRnd0Button{ nullptr };
   ClickButton* mRnd1Button{ nullptr };
   ClickButton* mRnd2Button{ nullptr };
   ClickButton* mRnd3Button{ nullptr };
   ClickButton* mClearButton{ nullptr };

   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };

   float mRndLengthChance{ 0.5f };
   FloatSlider* mRndLengthChanceSlider{ nullptr };
   float mRndLengthLo{ 1 };
   FloatSlider* mRndLengthLoSlider{ nullptr };
   float mRndLengthHi{ 24 };
   FloatSlider* mRndLengthHiSlider{ nullptr };
   float mRndOnsetChance{ 0.5f };
   FloatSlider* mRndOnsetChanceSlider{ nullptr };
   float mRndOnsetLo{ 1 };
   FloatSlider* mRndOnsetLoSlider{ nullptr };
   float mRndOnsetHi{ 12 };
   FloatSlider* mRndOnsetHiSlider{ nullptr };
   float mRndRotationChance{ 0.5f };
   FloatSlider* mRndRotationChanceSlider{ nullptr };
   float mRndRotationLo{ 0 };
   FloatSlider* mRndRotationLoSlider{ nullptr };
   float mRndRotationHi{ 4 };
   FloatSlider* mRndRotationHiSlider{ nullptr };
   float mRndOffsetChance{ 0.0f };
   FloatSlider* mRndOffsetChanceSlider{ nullptr };
   float mRndOffsetLo{ -0.1f };
   FloatSlider* mRndOffsetLoSlider{ nullptr };
   float mRndOffsetHi{ 0.1f };
   FloatSlider* mRndOffsetHiSlider{ nullptr };
   float mRndNoteChance{ 0.5f };
   FloatSlider* mRndNoteChanceSlider{ nullptr };
   float mRndOctaveLo{ 2 };
   FloatSlider* mRndOctaveLoSlider{ nullptr };
   float mRndOctaveHi{ 4 };
   FloatSlider* mRndOctaveHiSlider{ nullptr };

   void RandomizeLength(int ringIndex);
   void RandomizeOnset(int ringIndex);
   void RandomizeRotation(int ringIndex);
   void RandomizeOffset(int ringIndex);
   void RandomizeNote(int ringIndex, bool force);


   std::vector<EuclideanSequencerRing*> mEuclideanSequencerRings;
};
